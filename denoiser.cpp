// Copyright (c) 2021 Mateusz Wojt

#include "denoiser.h"

#include <iostream>
#include <iomanip>

DenoiserIop::DenoiserIop(Node *node) : PlanarIop(node)
{
	// initialize all members
	m_bHDR = true;
	m_bAffinity = false;
	m_numRuns = 1;
	m_numThreads = 0;
	m_maxMem = 0.f;
	m_width = m_height = 0;

	m_device = nullptr;
	m_filter = nullptr;

	m_defaultChannels = Mask_RGB;
	m_defaultNumberOfChannels = m_defaultChannels.size();
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
		m_filter.setImage("color", m_beautyPixels.data(), oidn::Format::Float3, m_width, m_height);
		m_filter.setImage("output", m_outputPixels.data(), oidn::Format::Float3, m_width, m_height);

		if (!m_albedoPixels.empty()) {
			m_filter.setImage("albedo", m_albedoPixels.data(), oidn::Format::Float3, m_width, m_height);
		}

		if (!m_normalPixels.empty()) {
			m_filter.setImage("normal", m_normalPixels.data(), oidn::Format::Float3, m_width, m_height);
		}

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
			clock_t start = clock(), diff;
			m_filter.execute();
			diff = clock() - start;
			int msec = diff * 1000 / CLOCKS_PER_SEC;
			sum += msec;
		}
		if (m_numRuns > 1)
		{
			sum /= m_numRuns;
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
	auto inputPlaneSize = m_width * m_height * m_defaultNumberOfChannels;
	m_outputPixels.resize(inputPlaneSize);

	// Clear vectors so it does not leave any residual pixels when disconnecting inputs
	m_albedoPixels.clear();
	m_normalPixels.clear();

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
		auto numPixels = inputPlane.bounds().area();

		// Allocate memory for each vector of pixels.
		if (i == 0)
			m_beautyPixels.resize(numPixels * m_defaultNumberOfChannels);
		if (i == 1)
			m_albedoPixels.resize(numPixels * m_defaultNumberOfChannels);
		if (i == 2)
			m_normalPixels.resize(numPixels * m_defaultNumberOfChannels);

		// Iterate over each channel and get pixel values.
		for (auto chanNo = 0; chanNo < m_defaultNumberOfChannels; chanNo++)
		{
			const float* indata = &inputPlane.readable()[chanStride * chanNo];

			for (auto x = 0; x < fr; x++)
			{
				for (auto y = 0; y < ft; y++) {
					if (i == 0)
						m_beautyPixels[(y * fr + x) * m_defaultNumberOfChannels + chanNo] = indata[y * fr + x];
					if (i == 1)
						m_albedoPixels[(y * fr + x) * m_defaultNumberOfChannels + chanNo] = indata[y * fr + x];
					if (i == 2)
						m_normalPixels[(y * fr + x) * m_defaultNumberOfChannels + chanNo] = indata[y * fr + x];
				}
			}
		}
	}

	{
		if (aborted() || cancelled())
			return;
		// OIDN
		setupOIDN();
		executeOIDN();
	}

	// Copy final output into the image plane.
	for (auto chanNo = 0; chanNo < m_defaultNumberOfChannels; chanNo++)
	{
		float* outdata = &plane.writable()[plane.chanStride() * chanNo];

		for (auto i = 0; i < m_width; i++)
		{
			for (auto j = 0; j < m_height; ++j) {
				outdata[j * m_width + i] = m_outputPixels[(j * m_width + i) * m_defaultNumberOfChannels + chanNo];
			}
		}
	}
}

static Iop *build(Node *node) { return new DenoiserIop(node); }
const Iop::Description DenoiserIop::d("Denoiser", "Filter/Denoiser", build);