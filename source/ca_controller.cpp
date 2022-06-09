//------------------------------------------------------------------------
// Copyright(c) 2022 schulvorsteher.
//------------------------------------------------------------------------

#include "ca_controller.h"
#include "ca_cids.h"
#include "vstgui/plugin-bindings/vst3editor.h"

#include "pluginterfaces/base/ibstream.h"
#include "base/source/fstreamer.h"
#include "params.h"
#include "mdaParameter.h"
#include "ca_circles.h"
#include "ca_sequence.h"
#include "vstgui/vstgui_uidescription.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

using namespace Steinberg;

namespace csse {

//------------------------------------------------------------------------
// CircularGateController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// ////////////////// IN Parameters ///////////////////////////////
	setKnobMode(Vst::kLinearMode);

	parameters.addParameter(
		new Vst::mda::ScaledParameter(
			STR16("Segments"), STR16("No"), segs_max, 1, Vst::ParameterInfo::kCanAutomate, kSegsId,segs_min, segs_max, true));
	
	// speed parameter
	parameters.addParameter(STR16("Speed"), nullptr, 9, 0, Vst::ParameterInfo::kCanAutomate, kSpeedId);

	// Stereo parameter
	parameters.addParameter(STR16("Stereo"), NULL, 2, .5f, Vst::ParameterInfo::kCanAutomate, kStereoId);

	// Blur parameter
	parameters.addParameter(STR16("Shape"), STR16("%"), 0, .5f, Vst::ParameterInfo::kCanAutomate, kBlurId);

	// THE Sequence
	parameters.addParameter(
		STR16("Sequence"), STR16("Sequence"), 0, 0, Vst::ParameterInfo::kCanAutomate, kSequenceId);// , 0, 4294967295));
			//new Vst::mda::ScaledParameter (

    ///////////////////////// SWITCHES //////////////////////////////////
	// Bypass parameter
	parameters.addParameter(
		STR16("Bypass"), nullptr, 1, 0, Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass, kBypassId);

	// Segments Up
	parameters.addParameter(
		STR16("SegmentsUp"), nullptr, 1, 0, Vst::ParameterInfo::kNoFlags, kSegsUpId);

	// Segments Down
	parameters.addParameter(
		STR16("SegmentsDown"), nullptr, 1, 0, Vst::ParameterInfo::kNoFlags, kSegsDownId);
	////////////////////////// OUT parameters ////////////////////////
	int32 stepCount = 0;
	Vst::ParamValue defaultVal = 0;
	int32 flags = Vst::ParameterInfo::kIsReadOnly;
	
	//// Segment OUT parameter
	//int32 tag = kClockId;
	//parameters.addParameter(STR16("ClockMessage"), nullptr, stepCount, defaultVal, flags, tag);
	
	// current segment OUT parameter
	int32 tag = kCurrSegmentId;
	parameters.addParameter(STR16("CurrSegment"), nullptr, stepCount, defaultVal, flags, tag);

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateController::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;

	IBStreamer streamer(state, kLittleEndian);

	float fval;
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	setParamNormalized(kSegsId, fval);
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	setParamNormalized(kSpeedId, fval);
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	setParamNormalized(kStereoId, fval);
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	setParamNormalized(kBlurId, fval);

	double dval;
	if (streamer.readDouble(dval) == false)
		return kResultFalse;
	setParamNormalized(kSequenceId, dval);

	int32 bypassState = 0;
	if (streamer.readInt32(bypassState) == false)
	{
		// could be an old version, continue
	}
	setParamNormalized(kBypassId, bypassState ? 1 : 0);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateController::setState (IBStream* state)
{
	// Here you get the state of the controller
	if (!state)
		return kResultFalse;

	IBStreamer streamer(state, kLittleEndian);
	//float fval;

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API CircularGateController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, "view", "ca_editor.uidesc");
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = kResultOk;
	float fsegs;
	if (tag == kSegsId) {
		fsegs = value;
	}

	if ((tag == kSegsDownId || tag == kSegsUpId) && value == 1.f)
	{
		float fsegs = getParamNormalized(kSegsId);
		float step = static_cast<float>(1) / (segs_max - segs_min + 1);
		float fmin = 1* step;
		if (fsegs < fmin)
		{
			fsegs = fmin;
		}

		float newVal = (tag == kSegsUpId) ? fsegs + step : fsegs - step;
		setParamNormalized(kSegsId, (float)newVal);


	}
	else
		result = EditControllerEx1::setParamNormalized (tag, value);

	return result;
}

//------------------------------------------------tag==------------------------
tresult PLUGIN_API CircularGateController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	// called by host to get a string for given normalized value of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamStringByValue (tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

//tresult PLUGIN_API CircularGateController::notify(Vst::IMessage* message) {
//	if (!message) { return kInvalidArgument; }
//	if (!strcmp(message->getMessageID(), "ClockMessage")) {
//		const void* dataOut;
//		double clockMessage;
//		if (message->getAttributes()->getFloat("clock", clockMessage) == kResultOk) {
//			int i = 1;
//		}
//		return kResultOk;
//	}
//	if (!strcmp(message->getMessageID(), "SegmentMessage")) {
//		const void* dataOut;
//		double segmentsMessage;
//		if (message->getAttributes()->getFloat("Segments", segmentsMessage) == kResultOk) {
//			int i = 1;
//		}
//		return kResultOk;
//	}
//	return EditControllerEx1::notify(message);
//}

//------------------------------------------------------------------------
} // namespace csse
