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
		new Vst::mda::ScaledParameter (
			STR16("Segments"), STR16("Segments"), 48, default_segs, Vst::ParameterInfo::kCanAutomate, kSegsId,4, 32, true));

	
	// THE Sequence
	parameters.addParameter(
		STR16("Sequence"), STR16("Sequence"), 0,  0, Vst::ParameterInfo::kCanAutomate, kSequenceId);

	// Bypass parameter
	parameters.addParameter(
		STR16("Bypass"), nullptr, 1, 0, Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass, kBypassId);

	////////////////////////// OUT parameters ////////////////////////
	int32 stepCount = 0;
	Vst::ParamValue defaultVal = 0;
	int32 flags = Vst::ParameterInfo::kIsReadOnly;
	
	// Segment OUT parameter
	int32 tag = kClockId;
	parameters.addParameter(STR16("ClockMessage"), nullptr, stepCount, defaultVal, flags, tag);
	
	// current segment OUT parameter
	tag = kCurrSegmentId;
	parameters.addParameter(STR16("CurrSegment"), nullptr, stepCount, defaultVal, flags, tag);

	// speed parameter
	parameters.addParameter(STR16("Speed"), nullptr, 0, 0, Vst::ParameterInfo::kCanAutomate, kSpeedId);

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
	setParamNormalized(kSequenceId, fval);
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	setParamNormalized(kSpeedId, fval);

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
	tresult result = EditControllerEx1::setParamNormalized (tag, value);
	return result;
}

//------------------------------------------------------------------------
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

tresult PLUGIN_API CircularGateController::notify(Vst::IMessage* message) {
	if (!message) { return kInvalidArgument; }
	if (!strcmp(message->getMessageID(), "ClockMessage")) {
		const void* dataOut;
		double clockMessage;
		if (message->getAttributes()->getFloat("clock", clockMessage) == kResultOk) {


			int i = 1;
			//if (this->getmy) 
			//	if (&view->getFFTView()) {
			//		view->getFFTView()->updateLineList(&view->getFFTView()->linelists, (std::vector<CDrawContext::LinePair>*)dataOut);
			//	}
			//}
		}
		return kResultOk;
	}
	return EditControllerEx1::notify(message);
}

//------------------------------------------------------------------------
} // namespace csse
