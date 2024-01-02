// Copyright (c) 2021-2024 Mateusz Wojt

#include "denoiser.h"

#include <iostream>
#include <iomanip>

static const char *const _deviceTypeNames[] = {
	"Auto",
	"CPU",
	"SYCL",
	"CUDA",
	"HIP",
	0
};

DenoiserIop::DenoiserIop(Node *node) : PlanarIop(node)
{
	// initialize all members
	m_bHDR = true;
	m_bAffinity = false;
	m_numRuns = 1;
	m_deviceType = 0; // Auto
	m_numThreads = 0;
	m_maxMem = 0.f;

	m_device = nullptr;
	m_filter = nullptr;

	m_colorBuffer = nullptr;
	m_albedoBuffer = nullptr;
	m_normalBuffer = nullptr;
	m_outputBuffer = nullptr;

	m_defaultChannels = Mask_RGB;
	m_defaultNumberOfChannels = m_defaultChannels.size();

	setupDevice();
};

void DenoiserIop::setupDevice()
{
	try
    {
		// create device
		m_device = oidnNewDevice(static_cast<OIDNDeviceType>(m_deviceType));
		const char *errorMessage;
		if (m_device.getError(errorMessage) != oidn::Error::None)
			throw std::runtime_error(errorMessage);

        // set device parameters
		m_device.set("numThreads", m_numThreads);
		m_device.set("setAffinity", m_bAffinity);

		// commit changes to the device
		m_device.commit();

		// This can be an expensive operation, so we initialize the filter here
		m_filter = m_device.newFilter("RT");
    }
    catch (const std::exception &e)
    {
        std::string message = e.what();
		error("[OIDN]: %s", message.c_str());
    }
}

void DenoiserIop::setupFilter()
{
	try
	{
		// set the images
		m_filter.setImage("color", m_colorBuffer, oidn::Format::Float3, m_width, m_height);
		m_filter.setImage("output", m_outputBuffer, oidn::Format::Float3, m_width, m_height);
		m_filter.setImage("albedo", m_albedoBuffer, oidn::Format::Float3, m_width, m_height);
		m_filter.setImage("normal", m_normalBuffer, oidn::Format::Float3, m_width, m_height);

		// set filter parameters
		m_filter.set("hdr", m_bHDR);
		m_filter.set("maxMemoryMB", m_maxMem);

		// commit changes to the filter
		m_filter.commit();
    }
    catch (const std::exception &e)
    {
        std::string message = e.what();
		error("[OIDN]: %s", message.c_str());
    }

}

void DenoiserIop::executeFilter()
{
	try
	{
		for (unsigned int i = 0; i < m_numRuns; i++)
		{
			m_filter.execute();
		}
	}
	catch (const std::exception &e)
	{
		std::string message = e.what();
		error("[OIDN]: %s", message.c_str());
	}
}

void DenoiserIop::knobs(Knob_Callback f)
{
	Enumeration_knob(f, &m_deviceType, _deviceTypeNames, "device", "Device Type");
	Tooltip(f,
			"Auto: device is selected automatically\n"
			"CPU: regular CPU denoising backend\n"
			"SYCL: \n"
			"CUDA: CUDA-based denoising backend\n"
			"HIP: ");
	
	Bool_knob(f, &m_bHDR, "hdr", "HDR");
	Tooltip(f, "Turn on if input image is high-dynamic range");
	SetFlags(f, Knob::STARTLINE);

	Bool_knob(f, &m_bAffinity, "affinity", "Enable thread affinity");
	Tooltip(f, "Enables thread affinitization (pinning software threads to hardware threads)\n"
	 		   "if it is necessary for achieving optimal performance");
	
	Float_knob(f, &m_maxMem, "maxmem", "Memory limit (MB)");
	Tooltip(f, "Limit the memory usage below the specified amount in megabytes.\n"
			   "0 = no memory limit.");
	
	Int_knob(f, &m_numRuns, "num_runs", "Number of runs");
	Tooltip(f, "Number of times the image will be fed into the denoise filter.");
}

int DenoiserIop::knob_changed(Knob* k)
{
	if (k->is("device"))
	{
		setupDevice();
		return 1;
	}
	return 0;
}

const char *DenoiserIop::input_label(int n, char *) const
{
	switch (n) {
	case 0:
		return "beauty";
	case 1:
		return "albedo";
	case 2:
		return "normal";
	default:
		return 0;
	}
}

void DenoiserIop::_validate(bool for_real)
{
	copy_info();
}

void DenoiserIop::getRequests(const Box & box, const ChannelSet & channels, int count, RequestOutput & reqData) const
{
	for (int i = 0, endI = getInputs().size(); i < endI; i++) {
		const ChannelSet readChannels = input(i)->info().channels();
		input(i)->request(readChannels, count);
	}
}

void DenoiserIop::renderStripe(ImagePlane &plane)
{
	if (aborted() || cancelled())
		return;

	const Box imageFormat = info().format();
	m_width = imageFormat.w();
	m_height = imageFormat.h();
	auto bufferSize = m_width * m_height * m_defaultNumberOfChannels * sizeof(float);

	{
		// Allocate memory for each buffer
		m_colorBuffer = m_device.newBuffer(bufferSize);
		m_albedoBuffer = m_device.newBuffer(bufferSize);
		m_normalBuffer = m_device.newBuffer(bufferSize);
		m_outputBuffer = m_device.newBuffer(bufferSize);
		setupFilter();
	}
	
	// Acquire pointers to the float data of each buffer
	float* colorPtr = static_cast<float*>(m_colorBuffer.getData());
	float* albedoPtr = static_cast<float*>(m_albedoBuffer.getData());
	float* normalPtr = static_cast<float*>(m_normalBuffer.getData());
	float* outputPtr = static_cast<float*>(m_outputBuffer.getData());

	if (!colorPtr || !albedoPtr || !normalPtr) {
    	// Handle error: Buffer allocation failed or pointers are null
		error("Buffer data is nullptr");
    	return;
	}

	for (auto i = 0; i < node_inputs(); ++i) {
		if (aborted() || cancelled())
			return;

		Iop* inputIop = dynamic_cast<Iop*>(input(i));
		
		// Can't figure out how to trigger this behavior, doesn't matter if input is connected or not.
		if (inputIop == nullptr) {
			continue;
		}

		// Validate input just in case before further processing.
		if (!inputIop->tryValidate(true)) {
			continue;
		}

		// Set our input bounding box, this is what our inputs can give us.
		Box imageBounds = inputIop->info();

		// We're going to clip it to our format.
		imageBounds.intersect(imageFormat);
		const int fx = imageBounds.x();
		const int fy = imageBounds.y();
		const int fr = imageBounds.r();
		const int ft = imageBounds.t();

		// Request input based on our format.
		inputIop->request(fx, fy, fr, ft, m_defaultChannels, 0);

		// Fetch plane from input into the image plane.
		ImagePlane inputPlane(imageBounds, false, m_defaultChannels, m_defaultNumberOfChannels);
		inputIop->fetchPlane(inputPlane);

		auto chanStride = inputPlane.chanStride();

		// Iterate over each channel and get pixel values.
		for (auto y = 0; y < ft; y++) {
			for (auto x = 0; x < fr; x++) {
				for (auto chanNo = 0; chanNo < m_defaultNumberOfChannels; chanNo++) {
					const float* indata = &inputPlane.readable()[chanStride * chanNo];
					size_t index = (y * fr + x) * m_defaultNumberOfChannels + chanNo;

					// Write to the appropriate buffer based on the channel number
					if (i == 0) {
						colorPtr[index] = indata[y * fr + x];
					}
					if (i == 1) {
						albedoPtr[index] = indata[y * fr + x];
					}
					if (i == 2) {
						normalPtr[index] = indata[y * fr + x];
					}
				}
			}
		}
	}

	{
		if (aborted() || cancelled())
			return;
		
		// Execute denoise filter
		executeFilter();
	}

	// Copy final output into the image plane.
	for (auto chanNo = 0; chanNo < m_defaultNumberOfChannels; chanNo++)
	{
		float* outdata = &plane.writable()[plane.chanStride() * chanNo];

		for (auto i = 0; i < m_width; i++) {
			for (auto j = 0; j < m_height; j++) {
				size_t index = (j * m_width + i) * m_defaultNumberOfChannels + chanNo;
				outdata[j * m_width + i] = outputPtr[index];
			}
		}
	}
}

static Iop *build(Node *node) { return new DenoiserIop(node); }
const Iop::Description DenoiserIop::d("Denoiser", "Filter/Denoiser", build);