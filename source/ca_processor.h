//------------------------------------------------------------------------
// Copyright(c) 2022 schulvorsteher.
//------------------------------------------------------------------------

#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "params.h"


namespace csse {

//------------------------------------------------------------------------
//  CircularGateProcessor
//------------------------------------------------------------------------
class CircularGateProcessor : public Steinberg::Vst::AudioEffect
{
public:
	CircularGateProcessor ();
	~CircularGateProcessor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return (Steinberg::Vst::IAudioProcessor*)new CircularGateProcessor; 
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	
	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

	/** send messages to other modules */
	void sendClock(float clockMessage);
	void sendNofSegments(float fSegs);

//------------------------------------------------------------------------
protected:
	// --------- nof Segments out param
	float fClockMessageOld = 0;
	float clockMessage = 0;
	float fCurrSegment = 0;
	float fCurrSegmentOld = 0;

	float fSegs = default_segs;
	int iSegs = 0;
	int iSegsOld = 0;
	float fSegsOld = 0;
	float fPw = default_pw;
	float fSwitch = 0;
	float fSequence = 0;
	int iSequence = 0;
	float fSequenceOld = 0;
	float fSpeed = 0;
	float fSpeedOld = 0;
	int alternator;
	double tempo;
	bool isSamplePosKnown;
	int lastRecordedBarNumber;
	int lastRecordedBeatInBar;
	long samplePos;
	long framesPerBeat;
	double samplerate;
	float fBarInfo;

	int displayed_segment_old = -1;

	bool reset_sequence;

	int allSamples;
	bool bBypass;
	std::vector<int> vSequence;
};

//------------------------------------------------------------------------
} // namespace csse
