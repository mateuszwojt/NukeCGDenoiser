#pragma warning(disable: 4996)

#include "denoiser.h"

#include <iostream>
#include <iomanip>

#include "DDImage/Black.h"

#define INPUT_BEAUTY 0
#define INPUT_ALBEDO 1
#define INPUT_NORMAL 2
#define DEBUG 1

// TODO: fix input_format to use the actual format of input (doesn't fetch correct size it's resized/reformatted)
// TODO: actually use albedo/normal to open call (right now it does nothing)
// TODO: optimize _request loop so it fetches image only once

static const char* const denoiseTypeNames[] = {
	"Intel",
	"Optix",
	0
};

DenoiserIop::DenoiserIop(Node* node) : Iop(node)
{
// initialize all members
    m_bHDR = true;
    m_bAffinity = false;
    m_blend = 0.f;
    m_denoiseType = 0;
    m_numRuns = 1;
    m_numThreads = 0;
    m_maxMem = 0.f;
    m_beautyWidth = m_beautyHeight = m_albedoWidth = m_albedoHeight = m_normalWidth = m_normalHeight = 0;

    m_optixContext = nullptr;
    m_beautyBuffer = nullptr;
    m_albedoBuffer = nullptr;
    m_normalBuffer = nullptr;
    m_outBuffer = nullptr;
    m_denoiserStage = nullptr;
    m_commandList = nullptr;

    m_device = nullptr;
    m_filter = nullptr;
};

void DenoiserIop::setupOptix()
{
	if (DEBUG)
	{
		std::cout << "Setting up Optix..." << std::endl;
	}

	try
	{
		// Create our optix context and image buffers
		m_optixContext = optix::Context::create();
		m_beautyBuffer = m_optixContext->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, m_beautyWidth, m_beautyHeight);
		m_albedoBuffer = m_optixContext->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, m_albedoWidth, m_albedoHeight);
		m_normalBuffer = m_optixContext->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, m_normalWidth, m_normalHeight);
		m_outBuffer = m_optixContext->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, m_beautyWidth, m_beautyHeight);

		m_pDevicePtr = (float*)m_beautyBuffer->map();
		unsigned int m_pixelIdx = 0;
		for (unsigned int y = 0; y < m_beautyHeight; y++)
			for (unsigned int x = 0; x < m_beautyWidth; x++)
			{
				memcpy(m_pDevicePtr, &m_beautyPixels[m_pixelIdx], sizeof(float) * 3);
				m_pDevicePtr += 4;
				m_pixelIdx += 3;
			}
		m_beautyBuffer->unmap();
		m_pDevicePtr = 0;

		// Setup the optix denoiser post processing stage
		m_denoiserStage = m_optixContext->createBuiltinPostProcessingStage("DLDenoiser");
		m_denoiserStage->declareVariable("input_buffer")->set(m_beautyBuffer);
		m_denoiserStage->declareVariable("output_buffer")->set(m_outBuffer);
		m_denoiserStage->declareVariable("blend")->setFloat(m_blend);
		m_denoiserStage->declareVariable("hdr")->setUint(m_bHDR);
		m_denoiserStage->declareVariable("maxmem")->setUint(static_cast<int>(m_maxMem));
		m_denoiserStage->declareVariable("input_albedo_buffer")->set(m_albedoBuffer);
		m_denoiserStage->declareVariable("input_normal_buffer")->set(m_normalBuffer);

		// Add the denoiser to the new optix command list
		m_commandList = m_optixContext->createCommandList();
		m_commandList->appendPostprocessingStage(m_denoiserStage, m_beautyWidth, m_beautyHeight);
		m_commandList->finalize();

		// Compile our context. I'm not sure if this is needed given there is no megakernal?
		m_optixContext->validate();
		m_optixContext->compile();
	}
	catch (const std::runtime_error &e)
	{
		std::string message = e.what();
		error("[OptiX]: %s", message.c_str());
	}
}

void DenoiserIop::executeOptix()
{
	if (DEBUG)
	{
		std::cout << "Executing OptiX denoise..." << std::endl;
	}

	try
	{
		// Execute denoise
		int sum = 0;
		for (unsigned int i = 0; i < m_numRuns; i++)
		{
			std::cout << "Denoising..." << std::endl;
			clock_t start = clock(), diff;
			commandList->execute();
			diff = clock() - start;
			int msec = diff * 1000 / CLOCKS_PER_SEC;
			if (DEBUG)
			{
				if (m_numRuns > 1)
					std::cout << "Denoising run " << i << " complete in " << msec / 1000 << "." << std::setfill('0') << std::setw(3) << msec % 1000 << " seconds" << std::endl;
				else
					std::cout << "Denoising complete in " << msec / 1000 << "." << std::setfill('0') << std::setw(3) << msec % 1000 << " seconds" << std::endl;
			}
			sum += msec;
		}

		if (m_numRuns > 1)
		{
			sum /= m_numRuns;
			if (DEBUG)
			{
				std::cout << "Denoising avg of " << m_numRuns << " complete in " << sum / 1000 << "." << std::setfill('0') << std::setw(3) << sum % 1000 << " seconds" << std::endl;
			}
		}
	}
	catch (const std::exception &e)
	{
		std::string message = e.what();
		error("[OptiX]: %s", message.c_str());
	}
}

void DenoiserIop::copyOptixFramebuffer()
{
	std::cout << "Copying framebuffer..." << std::endl;
	try
	{
		// Copy denoised image back to the cpu
		m_pDevicePtr = (float*)m_outBuffer->map();
		m_pixelIdx = 0;
		for (unsigned int y = 0; y < m_beautyHeight; y++)
			for (unsigned int x = 0; x < m_beautyWidth; x++)
			{
				memcpy(&m_outputPixels[m_pixelIdx], m_pDevicePtr, sizeof(float) * 3);
				m_pDevicePtr += 4;
				m_pixelIdx += 3;
			}
		m_outBuffer->unmap();
		m_pDevicePtr = 0;

		// Remove our gpu buffers
		m_beautyBuffer->destroy();
		m_normalBuffer->destroy();
		m_albedoBuffer->destroy();
		m_outBuffer->destroy();
		m_optixContext->destroy();
	}
	catch (const std::exception &e)
	{
		std::string message = e.what();
		error("[OptiX]: %s", message.c_str());
	}
}

void DenoiserIop::setupIntel()
{
	std::cout << "Initializing Intel denoise..." << std::endl;

	try
	{
		// create device
		device = oidn::newDevice();
		const char* errorMessage;
		if (device.getError(errorMessage) != oidn::Error::None)
			throw std::runtime_error(errorMessage);

		// set device parameters
		device.set("numThreads", num_threads);
		device.set("setAffinity", affinity);

		// commit changes to the device
		device.commit();

		// initialize filter
		filter = device.newFilter("RT");

		// set the images
		filter.setImage("color", (void*)&m_beautyPixels[0], oidn::Format::Float3, m_beautyWidth, m_beautyHeight);
		filter.setImage("output", (void*)&m_outputPixels[0], oidn::Format::Float3, m_beautyWidth, m_beautyHeight);

		// set filter parameters
		filter.set("hdr", hdr);
		filter.set("maxMemoryMB", maxmem);

		// commit changes to the filter
		filter.commit();

	}
	catch (const std::exception& e)
	{
		std::string message = e.what();
		error("[OIDN]: %s", message.c_str());
	}
}

void DenoiserIop::executeIntel()
{
	if (DEBUG)
	{
		std::cout << "Executing Intel denoise..." << std::endl;
	}

	try
	{
		int sum = 0;
		for (unsigned int i = 0; i < m_numRuns; i++)
		{
			if (DEBUG)
			{
				std::cout << "Denoising..." << std::endl;
			}
			clock_t start = clock(), diff;
			filter.execute();
			diff = clock() - start;
			int msec = diff * 1000 / CLOCKS_PER_SEC;
			if (DEBUG)
			{
				if (m_numRuns > 1)
					std::cout << "Denoising run " << i << " complete in " << msec / 1000 << "." << std::setfill('0') << std::setw(3) << msec % 1000 << " seconds" << std::endl;
				else
					std::cout << "Denoising complete in " << msec / 1000 << "." << std::setfill('0') << std::setw(3) << msec % 1000 << " seconds" << std::endl;
			}
			sum += msec;
		}
		if (m_numRuns > 1)
		{
			sum /= m_numRuns;
			if (DEBUG)
			{
				std::cout << "Denoising avg of " << m_numRuns << " complete in " << sum / 1000 << "." << std::setfill('0') << std::setw(3) << sum % 1000 << " seconds" << std::endl;
			}
		}
	}
	catch (const std::runtime_error& e)
	{
		std::string message = e.what();
		error("[OIDN]: %s", message.c_str());
	}
}

void DenoiserIop::knobs(Knob_Callback f)
{
	// TODO: split into sections for Intel/Optix-specific parameters
	Enumeration_knob(f, &m_denoiseType, denoiseTypeNames, "denoiseType");
	Tooltip(f,
		"Intel: Open Image Denoiser by Intel (CPU)\n"
		"OptiX: NVidia's denoiser (GPU)"
	);
	Bool_knob(f, &m_bHDR, "hdr"); SetFlags(f, Knob::STARTLINE);
	Bool_knob(f, &m_bAffinity, "affinity");
	Float_knob(f, &m_blend, "blend");
	Float_knob(f, &m_maxMem, "maxmem");
	Int_knob(f, &m_numRuns, "num_runs");
}

const char* DenoiserIop::input_label(int n, char*) const
{
	switch (n)
	{
	case INPUT_BEAUTY: return "beauty";
	case INPUT_ALBEDO: return "albedo";
	case INPUT_NORMAL: return "normal";
	}

	return 0;
}

void DenoiserIop::_validate(bool for_real)
{
	copy_info();
	merge_info();
}

void DenoiserIop::_request(int x, int y, int r, int t, ChannelMask channels, int count)
{
	for (int i = 0; i < MAX_INPUTS; ++i)
	{
		// TODO: do we need request all available channels? Could be enough to use just Mask_RGBA
		input(i)->request(x, y, r, t, channels, count);
	}
}

void DenoiserIop::_open()
{
	// initialize beauty AOV and output vector, check resolution
	if (!dynamic_cast<DD::Image::Black*>(input(INPUT_BEAUTY)))
	{
		Iop* op = dynamic_cast<Iop*>(Op::input(INPUT_BEAUTY));
		m_beautyWidth = op->input_format().width();
		m_beautyHeight = op->input_format().height();
		m_beautyPixels.resize(m_beautyWidth * m_beautyHeight * 3);
		m_outputPixels.resize(m_beautyWidth * m_beautyHeight * 3);
    }

	if (!dynamic_cast<DD::Image::Black*>(input(INPUT_NORMAL)))
	{
		Iop* op = dynamic_cast<Iop*>(Op::input(INPUT_NORMAL));
		m_normalWidth = op->input_format().width();
		m_normalHeight = op->input_format().height();
		if (m_normalWidth != m_beautyWidth || m_normalHeight != m_beautyHeight)
		{
			error("Normal input is not the same resolution as beauty. Please match the size.");
		}
		m_normalPixels.resize(m_normalWidth * m_normalHeight * 3);
	}

	if (!dynamic_cast<DD::Image::Black*>(input(INPUT_ALBEDO)))
	{
		Iop* op = dynamic_cast<Iop*>(Op::input(INPUT_ALBEDO));
		m_albedoWidth = op->input_format().width();
		m_albedoHeight = op->input_format().height();
		if (m_albedoWidth != m_beautyWidth || m_albedoHeight != m_beautyHeight)
		{
			error("Albedo input is not the same resolution as beauty. Please match the size.");
		}
		m_albedoPixels.resize(m_albedoWidth * m_albedoHeight * 3);
	}

	// TODO: iterate over all inputs (normal, albedo)
	//	for (int i = 0; i < MAX_INPUTS; ++i)
	//	{
	Iop* curInput = input(0);

	if (dynamic_cast<Black*>(input(0)))
	{
		return;
	}

	int width, height;
	width = curInput->input_format().width();
	height = curInput->input_format().height();

	// Channels from input and RGBA channel mask to intersect with
	ChannelSet chmask, inputChannels;
	chmask += Mask_RGB;
	inputChannels = curInput->channels();

	// For reading we want to access the input format, not bounding box.
	Format format = curInput->input_format();
	const int fx = format.x();
	const int fy = format.y();
	const int fr = format.r();
	const int ft = format.t();

	// Lock an area into the cache using an interest and release immediately
	Interest interest(*curInput, fx, fy, fr, ft, chmask, true);
	interest.unlock();

	std::vector<float> red_channel(width * height);
	std::vector<float> green_channel(width * height);
	std::vector<float> blue_channel(width * height);

    // Copy each channel row values into temp buffers
	foreach(channel, chmask)
	{
		if (inputChannels.contains(channel))
		{
			int currentRow = 0;

			for (int ry = fy; ry < ft; ry++)
			{
				Row row(fx, fr);
				row.get(*curInput, ry, fx, fr, chmask);

				const float *CUR = row[channel] + fx;

				switch (channel)
				{
				case Chan_Red:
					memcpy(&red_channel[ry * width], CUR, sizeof(float) * width);
				case Chan_Green:
					memcpy(&green_channel[ry * width], CUR, sizeof(float) * width);
				case Chan_Blue:
					memcpy(&blue_channel[ry * width], CUR, sizeof(float) * width);
				}
				currentRow++;
			}
		}
	}

	// contiguous packing of pixels
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			m_beautyPixels[(j * width + i) * 3 + 0] = red_channel[j * width + i];
			m_beautyPixels[(j * width + i) * 3 + 1] = green_channel[j * width + i];
			m_beautyPixels[(j * width + i) * 3 + 2] = blue_channel[j * width + i];
		}
	}

	if (m_denoiseType == 1)
	{
		setupOptix();
		executeOptix();
		copyOptixFramebuffer();
	}
	else
	{
		setupIntel();
		executeIntel();
	}
}

void DenoiserIop::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
	int size = m_outputPixels.size();
	float* pData = m_outputPixels.data();

	foreach(z, channels)
	{
		if (aborted()) { return; }
		float* outptr = out.writable(z) + x;

		for (int cur = x; cur < r; cur++)
		{
			if (z == Chan_Red || z == Chan_Blue || z == Chan_Green)
			{
				int offset = (y * m_beautyWidth + cur) * 3 + (z - 1);
				*outptr++ = pData[offset];
			}
			if (z == Chan_Alpha)
			{
				// TODO: copy original alpha back to the output
			}
		}
	}
}

static Iop* build(Node* node) { return new DenoiserIop(node); }
const Iop::Description DenoiserIop::d("Denoiser", "Filter/Denoiser", build);