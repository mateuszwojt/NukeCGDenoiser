#include "denoiser.h"

#include <iostream>
#include <iomanip>

#include <DDImage/Black.h>

#define INPUT_BEAUTY 0
#define INPUT_ALBEDO 1
#define INPUT_NORMAL 2
#define DEBUG 1

// TODO: fix input_format to use the actual format of input (doesn't fetch correct size it's resized/reformatted)
// TODO: actually use albedo/normal to open call (right now it does nothing)
// TODO: optimize _request loop so it fetches image only once

DenoiserIop::DenoiserIop(Node *node) : PlanarIop(node)
{
	// initialize all members
	m_bHDR = true;
	m_bAffinity = false;
	m_blend = 0.f;
	m_numRuns = 1;
	m_numThreads = 0;
	m_maxMem = 0.f;
	m_beautyWidth = m_beautyHeight = m_albedoWidth = m_albedoHeight = m_normalWidth = m_normalHeight = 0;

	m_device = nullptr;
	m_filter = nullptr;
};

void DenoiserIop::setupOIDN()
{
	try
	{
		// create device
		m_device = oidn::newDevice();
		const char *errorMessage;
		if (m_device.getError(errorMessage) != oidn::Error::None)
			throw std::runtime_error(errorMessage);

		// set device parameters
		m_device.set("numThreads", m_numThreads);
		m_device.set("setAffinity", m_bAffinity);

		// commit changes to the device
		m_device.commit();

		// initialize filter
		m_filter = m_device.newFilter("RT");

		// set the images
		m_filter.setImage("color", (void *)&m_beautyPixels[0], oidn::Format::Float3, m_beautyWidth, m_beautyHeight);
		m_filter.setImage("output", (void *)&m_outputPixels[0], oidn::Format::Float3, m_beautyWidth, m_beautyHeight);

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

void DenoiserIop::executeOIDN()
{
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
			m_filter.execute();
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
	catch (const std::runtime_error &e)
	{
		std::string message = e.what();
		error("[OIDN]: %s", message.c_str());
	}
}

void DenoiserIop::knobs(Knob_Callback f)
{
	Bool_knob(f, &m_bHDR, "hdr");
	SetFlags(f, Knob::STARTLINE);
	Bool_knob(f, &m_bAffinity, "affinity");
	Float_knob(f, &m_blend, "blend");
	Float_knob(f, &m_maxMem, "maxmem");
	Int_knob(f, &m_numRuns, "num_runs");
}

const char *DenoiserIop::input_label(int n, char *) const
{
	switch (n)
	{
	case INPUT_BEAUTY:
		return "beauty";
	case INPUT_ALBEDO:
		return "albedo";
	case INPUT_NORMAL:
		return "normal";
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
	if (!dynamic_cast<Black *>(input(INPUT_BEAUTY)))
	{
		Iop *op = dynamic_cast<Iop *>(Op::input(INPUT_BEAUTY));
		m_beautyWidth = op->input_format().width();
		m_beautyHeight = op->input_format().height();
		m_beautyPixels.resize(m_beautyWidth * m_beautyHeight * 3);
		m_outputPixels.resize(m_beautyWidth * m_beautyHeight * 3);
	}

	if (!dynamic_cast<Black *>(input(INPUT_NORMAL)))
	{
		Iop *op = dynamic_cast<Iop *>(Op::input(INPUT_NORMAL));
		m_normalWidth = op->input_format().width();
		m_normalHeight = op->input_format().height();
		if (m_normalWidth != m_beautyWidth || m_normalHeight != m_beautyHeight)
		{
			error("Normal input is not the same resolution as beauty. Please match the size.");
		}
		m_normalPixels.resize(m_normalWidth * m_normalHeight * 3);
	}

	if (!dynamic_cast<Black *>(input(INPUT_ALBEDO)))
	{
		Iop *op = dynamic_cast<Iop *>(Op::input(INPUT_ALBEDO));
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
	Iop *curInput = input(0);

	if (dynamic_cast<Black *>(input(0)))
	{
		return;
	}
}

void DenoiserIop::fetchPlane(ImagePlane &outputPlane)
{
	if (aborted())
		return;

	input0().fetchPlane(outputPlane);
	outputPlane.makeUnique();

	// Get imageplane size, different bboxes etc can mess this up, so needs more elaborate handling in practice
	int width = outputPlane.bounds().w();
	int height = outputPlane.bounds().h();

	float *destBuffer = nullptr;

	for (int channel = 0; channel < 3; ++channel)
	{
		destBuffer = &(outputPlane.writableAt(0, 0, channel));

		// Copy data from imageplane to other buffer
		memcpy(&m_beautyPixels[width * height * channel], destBuffer, sizeof(float) * width * height);
	}

	{
		// OIDN
		setupOIDN();
		executeOIDN();
	}

	// Copy data back
	for (int channel = 0; channel < 3; ++channel)
	{
		destBuffer = &(outputPlane.writableAt(0, 0, channel));
		memcpy(destBuffer, &m_beautyPixels[width * height * channel], sizeof(float) * width * height);
	}
}

void DenoiserIop::renderStripe(ImagePlane& plane)
{
	plane.makeWritable();

	foreach(z, outPlane.channels()) 
	{
		for (Box::iterator it = box.begin(); it != box.end(); it++) 
		{
			plane.writableAt(plane.chanNo(z), it.x, it.y) = 0.5;
		}
	}
}

static Iop *build(Node *node) { return new DenoiserIop(node); }
const Iop::Description DenoiserIop::d("Denoiser", "Filter/Denoiser", build);