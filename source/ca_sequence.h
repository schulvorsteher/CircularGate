//------------------------------------------------------------------------
// Copyright(c) 2021 csse.
//------------------------------------------------------------------------

#pragma once
#include "vector"
#include "pluginterfaces/vst/ivstmessage.h"

namespace csse {

//------------------------------------------------------------------------
// Sequencer.
//------------------------------------------------------------------------

class Sequence
{
public:
	static float getValue(
		int currSample, 
		int framesPerBeat, 
		std::vector<int> vSequence, 
		float fSpeed, 
		bool reset_sequence, 
		float fBlur,
		int& displayed_segment);

	static std::vector<int> sequenceToVector(int iSequence, int iSegs);
	static float vectorToSequence(std::vector<int> selection);
	static int sequenceToInt(float val, int vectorsize);

	static int denormalizeSegments(float fSegs);
	static float normalizeSegments(int segs);
	static float denormalizeSpeed(float fSpeed);
	//static int denormalizeSequence(float fSequence, int iSegs);
};
} // namespace csse

