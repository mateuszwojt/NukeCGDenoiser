#ifndef DENOISER_H
#define DENOISER_H

#include "DDImage/Iop.h"
#include "DDImage/Interest.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/Knob.h"
#include "DDImage/DDMath.h"

// #include <OpenImageDenoise/oidn.hpp>
// #include <optix_world.h>

#define MAX_INPUTS 3

static const char* const HELP = "CPU/GPU CG render denoiser based on Intel OpenImageDenoise and NVidia Optix libraries.";
static const char* const CLASS = "Denoiser";

using namespace DD::Image;

class DenoiserIop : public Iop
{
public:
// constructor
    DenoiserIop(Node* node);

// Nuke internal methods
	int minimum_inputs() const { return MAX_INPUTS; }
	int maximum_inputs() const { return 1; }

	void knobs(Knob_Callback f);

	void _validate(bool);
	void _request(int x, int y, int r, int t, ChannelMask channels, int count);
	void _open();

	void engine(int y, int x, int r, ChannelMask channels, Row& out);

	const char* input_label(int n, char*) const;
    static const Iop::Description d;

	const char* Class() const { return d.name; }
	const char* node_help() const { return HELP; }

// OptiX methods
	void setupOptix();
	void executeOptix();
	void copyOptixFramebuffer();

// Intel methods
	void setupIntel();
	void executeIntel();

// private class members
private:
	bool m_bHDR;
	bool m_bAffinity;

	float m_blend;
	float m_maxMem;

	int m_denoiseType;
	int m_numThreads;
	int m_numRuns;
	unsigned int m_beautyHeight, m_beautyWidth, m_albedoHeight, m_albedoWidth, m_normalHeight, m_normalWidth;

	float* m_pDevicePtr;
	unsigned int m_pixelIdx;

// Optix class members
	optix::Context m_optixContext;
	optix::Buffer m_beautyBuffer;
	optix::Buffer m_albedoBuffer;
	optix::Buffer m_normalBuffer;
	optix::Buffer m_outBuffer;
	optix::PostprocessingStage m_denoiserStage;
	optix::CommandList m_commandList;

// Intel class members
	oidn::DeviceRef m_device;
	oidn::FilterRef m_filter;

// buffers
	std::vector<float> m_beautyPixels;
	std::vector<float> m_albedoPixels;
	std::vector<float> m_normalPixels;
	std::vector<float> m_outputPixels;
};

#endif // DENOISER_H