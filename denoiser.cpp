#include "denoiser.h"

#include <iostream>
#include <iomanip>

#include <DDImage/Black.h>
#include <DDImage/LUT.h>

#define DEBUG 1

DenoiserIop::DenoiserIop(Node *node) : PlanarIop(node)
{
	// initialize all members
	m_bHDR = true;
	m_bAffinity = false;
	m_numRuns = 1;
	m_numThreads = 0;
	m_maxMem = 0.f;
	m_beautyWidth = m_beautyHeight = 0;

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
	Float_knob(f, &m_maxMem, "maxmem");
	Int_knob(f, &m_numRuns, "num_runs");
}

const char *DenoiserIop::input_label(int n, char *) const
{
	if (n == 0)
		return "beauty";

	return 0;
}

void DenoiserIop::_validate(bool for_real)
{
	copy_info();
}

void DenoiserIop::getRequests(const Box & box, const ChannelSet & channels, int count, RequestOutput & reqData) const
{
	reqData.request(&input0(), box, channels, count);
}

void DenoiserIop::renderStripe(ImagePlane &plane)
{
	if (aborted() || cancelled())
		return;

	input0().fetchPlane(plane);

	const ChannelSet channels = plane.channels();
	const size_t numComponents = 4;
	const size_t numPixels = plane.bounds().area();
	m_beautyWidth = plane.bounds().w();
	m_beautyHeight = plane.bounds().h();
	m_beautyPixels.resize(numPixels * numComponents);
	m_outputPixels.resize(numPixels * numComponents);

	foreach(z, channels)
	{
		auto chanNo = plane.chanNo(z);
		auto chanStride = plane.chanStride();
		const float* indata = &plane.readable()[chanStride * chanNo];
		memcpy(&m_beautyPixels[chanStride * chanNo], indata, sizeof(float) * chanStride);
	}

	{
		if (aborted() || cancelled())
			return;
		// OIDN
		setupOIDN();
		executeOIDN();
	}

	plane.makeWritable();
	float* data = plane.writable();

	foreach(z, channels)
	{
		auto chanOffset = colourIndex(z);
		memcpy(data + plane.chanStride() * chanOffset, &m_outputPixels[plane.chanStride() * chanOffset], 
			sizeof(float) * plane.chanStride());
	}
}

static Iop *build(Node *node) { return new DenoiserIop(node); }
const Iop::Description DenoiserIop::d("Denoiser", "Filter/Denoiser", build);