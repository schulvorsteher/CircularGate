//------------------------------------------------------------------------
// Copyright(c) 2022 schulvorsteher.
//------------------------------------------------------------------------

#include "ca_processor.h"
#include "ca_cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/ivstmessage.h"

#include "params.h"
#include "ca_sequence.h"
#include <iostream>
#include <cstdio>
#include "mdaParameter.h"
#include <iomanip>


using namespace Steinberg;

namespace csse {
//------------------------------------------------------------------------
// CircularGateProcessor
//------------------------------------------------------------------------
CircularGateProcessor::CircularGateProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kCircularGateControllerUID);
}

//------------------------------------------------------------------------
CircularGateProcessor::~CircularGateProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instanciated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 ("Event In"), 1);

	alternator = 0;
	tempo = 0;
	isSamplePosKnown = false;
	lastRecordedBarNumber = 0;
	lastRecordedBeatInBar = 0;
	samplePos = 0;
	
	framesPerBeat = 0;


	allSamples = 0;
	bBypass = false;
	fCurrSegmentOld = 0;
	fBarInfo = 0; 

	reset_sequence = true;


	//freopen("/output.log", "w", stdout);
	//freopen("/outerr.log", "w", stderr);
	//std::cout << std::fixed << std::setprecision(10);

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::terminate ()
{
	// Here the Plug-in will be de-instanciated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------
    if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            if (auto* paramQueue = data.inputParameterChanges->getParameterData (index))
            {
                
				Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
				if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
				{
					switch (paramQueue->getParameterId())
					{
					case kSegsId:
						fSegs = (float)value;
						//controller.setParamNormalized(kSegsId, fsegs);
						//std::cout << "fSegs: ";
						//std::cout << fSegs << std::endl;
						break;

					case kPwId:
						fPw = (float)value;
						//std::cout << "fPw: ";
						//std::cout << fPw << std::endl;
						break;
					case kSwitchId:
						fSwitch = (float)value;
						//std::cout << "fSwitch: ";
						//std::cout << fSwitch << std::endl;
						break;
					case kSequenceId:
						fSequence = (float)value;
						//floatToVector(fSequence);
						//std::cout << "fSequence: ";
						//std::cout << fSequence << std::endl;
						//for (int s : selection)
						//	std::cout << s << " ";
						//std::cout << std::endl;
						break;
					case kSpeedId:
						fSpeed = (float)value;
						break;
					}
				}
			}
		}
	}
	// get tempo / time signature
	
	//--- Here you have to implement your processing
	if (data.numInputs == 0 || data.numOutputs == 0)
	{
		return kResultOk;
	}

	float fVuL = 0;
	float fVuR = 0;
	float fClockMessage = 0;
	
	float fSpeedDenormalized = Sequence::denormalizeSpeed(fSpeed);

	

	if (data.processContext != NULL)
	{
		//
		uint32	state = data.processContext->state;

		/*
		std::cout << "BarPositionMusic: ";
		std::cout << data.processContext->barPositionMusic << std::endl;
		//std::cout << "Chord: ";
		//std::cout << data.processContext->chord << std::endl;
		std::cout << "continousTimeSamples: ";
		std::cout << data.processContext->continousTimeSamples << std::endl;
		std::cout << "cycleEndMusic: ";
		std::cout << data.processContext->cycleEndMusic << std::endl;
		std::cout << "cycleStartMusic: ";
		std::cout << data.processContext->cycleStartMusic << std::endl;
		//std::cout << "frameRate: ";
		//std::cout << data.processContext->frameRate << std::endl;
		std::cout << "kBarPositionValid: ";
		std::cout << data.processContext->kBarPositionValid << std::endl;
		std::cout << "kChordValid: ";
		std::cout << data.processContext->kChordValid << std::endl;
		std::cout << "kClockValid: ";
		std::cout << data.processContext->kClockValid << std::endl;
		std::cout << "kContTimeValid: ";
		std::cout << data.processContext->kContTimeValid << std::endl;
		std::cout << "kCycleActive: ";
		std::cout << data.processContext->kCycleActive << std::endl;
		std::cout << "kCycleValid: ";
		std::cout << data.processContext->kCycleValid << std::endl;
		std::cout << "kPlaying: ";
		std::cout << data.processContext->kPlaying << std::endl;
		std::cout << "kProjectTimeMusicValid: ";
		std::cout << data.processContext->kProjectTimeMusicValid << std::endl;
		std::cout << "kRecording: ";
		std::cout << data.processContext->kRecording << std::endl;
		std::cout << "kSmpteValid: ";
		std::cout << data.processContext->kSmpteValid << std::endl;
		std::cout << "kSystemTimeValid: ";
		std::cout << data.processContext->kSystemTimeValid << std::endl;
		std::cout << "kTempoValid: ";
		std::cout << data.processContext->kTempoValid << std::endl;
		std::cout << "kTimeSigValid: ";
		std::cout << data.processContext->kTimeSigValid << std::endl;
		std::cout << "projectTimeMusic: ";
		std::cout << data.processContext->projectTimeMusic << std::endl;
		std::cout << "projectTimeSamples: ";
		std::cout << data.processContext->projectTimeSamples << std::endl;
		std::cout << "sampleRate: ";
		std::cout << data.processContext->sampleRate << std::endl;
		std::cout << "samplesToNextClock: ";
		std::cout << data.processContext->samplesToNextClock << std::endl;
		std::cout << "smpteOffsetSubframes: ";
		std::cout << data.processContext->smpteOffsetSubframes << std::endl;
		std::cout << "state: ";
		std::cout << data.processContext->state << std::endl;
		std::cout << "systemTime: ";
		std::cout << data.processContext->systemTime << std::endl;
		std::cout << "tempo: ";
		std::cout << data.processContext->tempo << std::endl;
		std::cout << "timeSigDenominator: ";
		std::cout << data.processContext->timeSigDenominator << std::endl;
		std::cout << "timeSigNumerator: ";
		std::cout << data.processContext->timeSigNumerator << std::endl;
		*/
		if (data.processContext)// && data.processContext->state & Vst::ProcessContext::kTempoValid)
		{
			if (data.processContext->tempo != tempo)
			{
				tempo = data.processContext->tempo;
				samplerate = data.processContext->sampleRate;
				framesPerBeat = samplerate / (tempo / 60);// *fSpeedDenormalized;
			}
		}
		isSamplePosKnown = true;
		if (isSamplePosKnown &&
			((state & Vst::ProcessContext::kPlaying) != 0) &&
			((state & Vst::ProcessContext::kProjectTimeMusicValid) != 0) &&
			((state & Vst::ProcessContext::kBarPositionValid) != 0) &&
			((state & Vst::ProcessContext::kTempoValid) != 0) &&
			((state & Vst::ProcessContext::kTimeSigValid) != 0))
		{
			// Compute last bar#, and fraction of a bar since then
			float		beatsPerQNote = (float)(data.processContext->timeSigDenominator) / 4.0;
			float		barsPerQNote = beatsPerQNote / data.processContext->timeSigNumerator;
			float		lastBarNumberFloat = data.processContext->barPositionMusic * barsPerQNote; //barPositionMusic is in qNotes!
			float		barNumberIntPart = floor(lastBarNumberFloat); // -2.3 becomes -3.0, as we want!
			int32		lastBarNumber = (int32)barNumberIntPart;

			//clockMessage = lastBarNumberFloat;

			/* TODO: what if this is not zero?
			float		barNumberFraction		= (lastBarNumberFloat >= 0) ?
														lastBarNumberFloat - barNumberIntPart :	// if >= 0
														barNumberIntPart - lastBarNumberFloat;		// if < 0
			*/

			// Compute last beat# in current bar, and fraction of a beat since then
			float		qNotesBackToLastBar = data.processContext->projectTimeMusic - data.processContext->barPositionMusic;
			float		currentBeatFloat = qNotesBackToLastBar * beatsPerQNote;
			float		currentBeatIntPart = floor(currentBeatFloat); // always >= 0
			float		currentBeatFraction = currentBeatFloat - currentBeatIntPart;
			uint16	lastBeatNumber = (uint16)currentBeatIntPart;

			// Increment bar and beat, since calculations is 0-based, but numbers are 1-based!
			lastBarNumber++;
			lastBeatNumber++;
			// Are we at a new bar or beat?

			if (lastBarNumber != lastRecordedBarNumber)
			{
				reset_sequence = true;
			}

			if ((lastBarNumber != lastRecordedBarNumber) || (lastBeatNumber != lastRecordedBeatInBar))
			{
				// Yes; Record the new bar/beat numbers
				lastRecordedBarNumber = lastBarNumber;
				lastRecordedBeatInBar = lastBeatNumber;

				// Now, we have the most recent bar# and the most recent beat#, 
				// and the fraction of a beat since the last beat.
				// We need to determine how long ago the last beat occurred, in samples.
				// To do that, we need to determine the samples per beat,
				// which is the samples per second (sample rate) divided by the beats per second.
				//	The beats per second equals the quarter notes per second times the beats per quarter note.
				//	The quarter notes per second equals the tempo divided by 60, and
				//	the beats per quarter note equals time signature denominator divided by 4. So...
				float		qNotesPerSecond = data.processContext->tempo / 60.0;
				float		beatsPerSecond = qNotesPerSecond * beatsPerQNote;
				float		samplesPerBeat = data.processContext->sampleRate / beatsPerSecond;
				//
				float		samplesSinceLastBeat = samplesPerBeat * currentBeatFraction;
				uint16	samplesSinceBeatInt = (uint16)samplesSinceLastBeat;
				long		sampleAtBeat = samplePos - samplesSinceBeatInt;
				//	Now we have the sample position of the last beat, which we use to mark it on our sample-based graph	}
				//framesPerBeat = samplerate / qNotesPerSecond;
				fBarInfo    = 10 * lastRecordedBarNumber + lastRecordedBeatInBar;
			}
		}
	}

	// segments + sequence
	if ((fSegs != fSegsOld) || (fSequence != fSequenceOld))
	{
		if (fSegs != fSegsOld) 
		{
			sendNofSegments(fSegs);
		}
		iSegs = Sequence::denormalizeSegments(fSegs);
		
		//iSegs = 8;
		iSequence = Sequence::sequenceToInt(fSequence, iSegs);

		vSequence = Sequence::sequenceToVector(iSequence, iSegs);
		
		fSegsOld = fSegs;
		fSequenceOld = fSequence;
	}

	if (fSpeedOld != fSpeedDenormalized)
	{
		framesPerBeat = samplerate / (tempo / 60);// *fSpeedDenormalized;
		fSpeedOld = fSpeedDenormalized;
	}

	int displayed_segment;
	
	Steinberg::Vst::IParameterChanges* outParamChanges = data.outputParameterChanges;

	int32 numChannels = data.inputs[0].numChannels;
	Vst::Sample32** in = data.inputs[0].channelBuffers32;
	Vst::Sample32** out = data.outputs[0].channelBuffers32;
	
	for (int32 ch = 0; ch < numChannels; ch++)
	{
		Vst::Sample32* pIn = in[ch];
		Vst::Sample32* pOut = out[ch];
		Vst::Sample32 tmp;
		
		
		for (int32 i = 0;i<data.numSamples;i++)
		{
			tmp = *pIn;

			// waveform
			int waveform = Sequence::waveform_sine;
			if (fSwitch == 1.f) waveform = Sequence::waveform_saw;
			else if (fSwitch == .5f)waveform = Sequence::waveform_square;

			float y = Sequence::getValue(allSamples, framesPerBeat, waveform, fPw, vSequence, fSpeedDenormalized, reset_sequence, /*out*/ displayed_segment);
			reset_sequence = false;

			tmp *= y;

			
			if (framesPerBeat > 0 && allSamples % framesPerBeat == 0)
			{
				clockMessage++;
				//fCurrSegment = (int)clockMessage % (vSequence.size());
				//sendClock(clockMessage++);
			}

			if (displayed_segment >= 0)
			{
				if (displayed_segment != displayed_segment_old)
				{
					displayed_segment_old = displayed_segment;
					fCurrSegment = displayed_segment;
					//std::cout << "prc_bar:" << bar << std::endl;

					fClockMessage = 100 * fBarInfo + displayed_segment;// clockMessage;
					if (outParamChanges && fClockMessageOld != fClockMessage)
					{
						int32 index = 0;
						Steinberg::Vst::IParamValueQueue* paramQueue = outParamChanges->addParameterData(kClockId, index);
						if (paramQueue)
						{
							int32 index2 = 0;
							int mess = (int)fClockMessage;// % 100;
							paramQueue->addPoint(0, (float)mess / 10000, index2);
						}
					}
					fClockMessageOld = fClockMessage;
				}
			}
			allSamples++;
		
			*pOut = tmp;
			pIn++;
			pOut++;
		}


	}


	if (outParamChanges && fCurrSegment != fCurrSegmentOld)
	{
		int32 index = 0;
		Steinberg::Vst::IParamValueQueue* paramQueue = outParamChanges->addParameterData(kCurrSegmentId, index);
		if (paramQueue)
		{
			int32 index2 = 0;
			paramQueue->addPoint(0, (float)fCurrSegment / 100, index2);
		}
		fCurrSegmentOld = fCurrSegment;
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	/* if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue; */

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	float fval;
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	fSegs = fval;
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	fPw = fval;
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	fSwitch = fval;
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	fSequence = fval;
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	fSpeed = fval;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);
	streamer.writeFloat(fSegs);
	streamer.writeFloat(fPw);
	streamer.writeFloat(fSwitch);
	streamer.writeFloat(fSequence);
	streamer.writeFloat(fSpeed);
	
	return kResultOk;
}

//------------------------------------------------------------------------

void CircularGateProcessor::sendClock(float clockMessage)
{
	Vst::IMessage* message = allocateMessage();
	if (message)
	{
		FReleaser msgReleaser(message);
		message->setMessageID("ClockMessage");

		message->getAttributes()->setFloat("clock", clockMessage);
		sendMessage(message);
	}

}

void CircularGateProcessor::sendNofSegments(float fSegs)
{
	//---send a binary message
	Vst::IMessage* message = allocateMessage();
	if (message)
	{
		FReleaser msgReleaser(message);
		message->setMessageID("BinaryMessage");

		message->getAttributes()->setFloat("MyData", fSegs);
		sendMessage(message);
	}

}

//------------------------------------------------------------------------
} // namespace csse
