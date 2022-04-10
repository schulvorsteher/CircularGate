//------------------------------------------------------------------------
// Copyright(c) 2021 csse.
//------------------------------------------------------------------------

#include "ca_sequence.h"
#include <cmath>
#include "vector"
//#include <cstdio>
#include <iostream>
#include "vstgui/vstgui.h"

namespace csse {
	

	float y = 0;
	float x = 0;
	float currSegOld = 0;
	long deltasample = 0;
	
	float Sequence::getValue(
		int currSample, 
		int framesPerBeat, 
		std::vector<int> vSequence, 
		float fSpeed, 
		bool reset_sequence, 
		int& displayed_segment)
	{
		framesPerBeat *= fSpeed;
		if (framesPerBeat > 0)
		{
			int delta = (currSample - deltasample) % framesPerBeat;

			// beginning of beat -> syncronize array
			if (reset_sequence) {
				deltasample = currSample;
				reset_sequence = false;
			}
			if (deltasample < 0)
			{
				reset_sequence = true;  // can't be happening
			}

			if (delta == 0)
			{
				y = 0;
				x = 0;
			}
			else
			{
				// position in cycle
				x = (float)delta / framesPerBeat;
			}

			y = 1;
			//std::cout << "iSpeed:" << iSpeed << " x*speed:" << x * iSpeed << std::endl;

			int iSegs = vSequence.size();
			if (iSegs > 0)
			{
				int currSeg = (int)(x * iSegs);
				int seg = vSequence[currSeg];
				if (currSeg != currSegOld) 
				{
					currSegOld = currSeg;
					displayed_segment = currSeg;
				}
				y *= seg;
			}
		}
		return y;
	}

	int Sequence::denormalizeSegments(float segs)
	{
		//return std::min(16, 4 + (int)((segs * (12 + 1))));
		return std::min(32, 4 + (int)((segs * (28 + 1))));
	}

	//int Sequence::denormalizeSequence(float sequence, int segs)
	//{
	//	int max = pow(2, segs) - 1;
	//	return (int)(sequence * max);
	//}

	std::vector<int> Sequence::sequenceToVector(int iSequence, int iSegs)
	{
		std::vector<int> selection;
		int val = iSequence;
		for (int i = 0; i < iSegs; i++)
		{
			int v = val % 2;
			selection.push_back(v);
			val -= v;
			val /= 2;
		}
		return selection;
	}

	float Sequence::vectorToSequence(std::vector<int> selection)
	{
		int iSequence = 0;
		for (int i = 0; i < selection.size(); i++)
		{
			if (selection[i] == 1)
				iSequence += pow(2, i);
		}
		float fSequence = iSequence / pow(10, ceil((float)selection.size() / 3));
		return fSequence;
	}

	int Sequence::sequenceToInt(float val, int vectorsize)
	{
		int factor = pow(10, std::ceil((float)vectorsize / 3));
		int retval = round(val * factor);
		return retval;
	}

	float Sequence::denormalizeSpeed(float fSpeed)
	{
		int stage = fSpeed * 8;
		float speedlist[] = { 64.,32.,16.,8.,4.,2.,1.,.5,.25 };// , .25, .125, .0625, .03125
	
		return speedlist[stage];
	}




} // namespace csse