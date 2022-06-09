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
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"


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
	fCurrSegmentOld = 0;
	fBarInfo = 0; 

	reset_sequence = true;

	mBypass = false;


	//freopen("/output.log", "w", stdout);
	//freopen("/outerr.log", "w", stderr);
	//std::cout << std::fixed << std::setprecision(12);

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
				float val;
				Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
				if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
				{
					switch (paramQueue->getParameterId())
					{
					case kSegsId:
						fSegs = (float)value;
						break;

					case kSpeedId:
						fSpeed = (float)value;
						break;

					case kStereoId:
						fStereo = (float)value;
						break;

					case kBlurId:
						fBlur = (float)value;
						break;

					case kSequenceId:
						dSequence = (double)value;
						break;

					case kBypassId:
						mBypass = (value > .5f);
						break;

					case kSegsUpId:
						break;

					case kSegsDownId:
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
				//reset_sequence = true;
				
				// { 64.,32.,16.,8.,4.,2.,1.,.5,.25 }
				int speed = fSpeedDenormalized / 8;
				if (speed < 2) {
					reset_sequence = true;
				}
				else {
					if (speed == barNumberIntPart) reset_sequence = true;
				}
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
	if ((fSegs != fSegsOld) || (dSequence != fSequenceOld))
	{
		if (fSegs != fSegsOld) 
		{
			iSegs = Sequence::denormalizeSegments(fSegs);
			//std::cout << "ca_processor 259: iSegs:     ";
			//std::cout << iSegs << std::endl;
		}

		//std::cout << "ca_processor 263: iSegs:  (" << fSegs << ")";
		//std::cout << iSegs << std::endl;

		//std::cout << "ca_processor 266: dSequence: ";
		//std::cout << dSequence << std::endl;

		iSequence = Sequence::sequenceToInt(dSequence, iSegs);
		//std::cout << "ca_processor 270: iSequence: ";
		//std::cout << iSequence << std::endl;

		vSequence = Sequence::sequenceToVector(iSequence, iSegs);
		//std::cout << "ca_process 274: ";
		//for (int s : vSequence)
		//	if (s == 0)std::cout << ".";
		//	else std::cout << "X";	
		//std::cout << "(" << vSequence.size() << ")" << std::endl;

		fSegsOld = fSegs;
		fSequenceOld = dSequence;
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


	if (mBypass)
	{
		uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
		for (int32 i = 0; i < numChannels; i++)
		{
			// do not need to be copied if the buffers are the same
			if (in[i] != out[i])
			{
				memcpy(out[i], in[i], sampleFramesSize);
			}
		}
	}
	else
	{
		for (int32 ch = 0; ch < numChannels; ch++)
		{
			Vst::Sample32* pIn = in[ch];
			Vst::Sample32* pOut = out[ch];
			Vst::Sample32 tmp;


			for (int32 i = 0; i < data.numSamples; i++)
			{
				tmp = *pIn;

				float y = Sequence::getValue(allSamples, framesPerBeat, vSequence, fSpeedDenormalized, reset_sequence, fBlur, /*out*/ displayed_segment);
				reset_sequence = false;

				if (fStereo <.49f)
				{
					if (ch == 0)
						tmp *= y;
					else
						tmp *= (1 - y) * (1 - 2 * fStereo) + 2 * fStereo * y;
				}
				else if (fStereo < .51f)
				{
					tmp *= y; // for all channels
				}
				else // fStereo == 1.0f
				{
					if (ch == 0)
						tmp *= y * 2 * (.5f - fStereo / 2) + (2 * fStereo * (1 - y) / 2);
					else
						tmp *= y;
				}

				//if (fStereo <.45f)
				//{
				//	tmp *= ch == 0 ? y : (1 - y);
				//}
				//else if (fStereo < .550f)
				//{
				//	tmp *= y; // for all channels
				//}
				//else // fStereo == 1.0f
				//{
				//	tmp *= ch == 0 ? (1 - y) : y;
				//}

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

						//fClockMessage = 100 * fBarInfo + displayed_segment;// clockMessage;
						//if (outParamChanges && fClockMessageOld != fClockMessage)
						//{
						//	int32 index = 0;
						//	Steinberg::Vst::IParamValueQueue* paramQueue = outParamChanges->addParameterData(kClockId, index);
						//	if (paramQueue)
						//	{
						//		int32 index2 = 0;
						//		int mess = (int)fClockMessage;// % 100;
						//		paramQueue->addPoint(0, (float)mess / 10000, index2);
						//	}
						//}
						//fClockMessageOld = fClockMessage;
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
	fSpeed = fval;
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	fStereo = fval;
	if (streamer.readFloat(fval) == false)
		return kResultFalse;
	fBlur = fval;

	double dval;
	if (streamer.readDouble(dval) == false)
		return kResultFalse;
	dSequence = dval;

	int32 savedBypass = 0;
	if (streamer.readInt32(savedBypass) == false)
	{
		// could be an old version, continue))
	}
	mBypass = savedBypass > 0;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API CircularGateProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);


	streamer.writeFloat(fSegs);
	streamer.writeFloat(fSpeed);
	streamer.writeFloat(fStereo);
	streamer.writeFloat(fBlur);

	streamer.writeDouble(dSequence);
	
	streamer.writeInt32(mBypass ? 1 : 0);

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
		message->setMessageID("SegmentMessage");

		message->getAttributes()->setFloat("Segments", fSegs);
		sendMessage(message);
	}

}

//------------------------------------------------------------------------
} // namespace csse
