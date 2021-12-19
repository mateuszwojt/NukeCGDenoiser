#include <DDImage/PlanarIop.h>
#include <DDImage/Interest.h>
#include <DDImage/Row.h>
#include <DDImage/Knobs.h>
#include <DDImage/Knob.h>
#include <DDImage/DDMath.h>

#include <OpenImageDenoise/oidn.hpp>

static const char *const HELP = "CG render denoiser based on Intel OpenImageDenoise library";
static const char *const CLASS = "Denoiser";

using namespace DD::Image;

class DenoiserIop : public PlanarIop
{
public:
	// constructor
	DenoiserIop(Node *node);

	// Nuke internal methods
	int minimum_inputs() const { return 1; }
	int maximum_inputs() const { return 3; }

	PackedPreference packedPreference() const { return ePackedPreferenceUnpacked; }

	void knobs(Knob_Callback f);

	void _validate(bool);

	void getRequests(const Box& box, const ChannelSet& channels, int count, RequestOutput &reqData) const;
	virtual void renderStripe(ImagePlane& plane);

	bool useStripes() const { return false; }

	bool renderFullPlanes() const { return true; }

	const char *input_label(int n, char *) const;
	static const Iop::Description d;

	const char *Class() const { return d.name; }
	const char *node_help() const { return HELP; }

	// Intel methods
	void setupOIDN();
	void executeOIDN();

	// private class members
private:
	bool m_bHDR;
	bool m_bAffinity;

	float m_maxMem;

	int m_numThreads;
	int m_numRuns;
	unsigned int m_width, m_height;

	ChannelSet kDefaultChannels;
	int kDefaultNumberOfChannels;

	// OIDN class members
	oidn::DeviceRef m_device;
	oidn::FilterRef m_filter;

	// buffers
	std::vector<float> m_beautyPixels;
	std::vector<float> m_albedoPixels;
	std::vector<float> m_normalPixels;
	std::vector<float> m_outputPixels;
};