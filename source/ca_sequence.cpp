//------------------------------------------------------------------------
// Copyright(c) 2021 csse.
//------------------------------------------------------------------------

#include "ca_sequence.h"
#include <cmath>
#include "vector"
//#include <cstdio>
#include <iostream>
#include "vstgui/vstgui.h"
#include "params.h"

using namespace Steinberg;
namespace csse {
	

	float y = 0;
	float x = 0;
	float dy = 0;
	float currSegOld = 0;
	long deltasample = 0;
	long deltaseg = 0;
	int segvalold = 0;
	
	float Sequence::getValue(
		int currSample, 
		int framesPerBeat, 
		std::vector<int> vSequence, 
		float fSpeed, 
		bool reset_sequence, 
		float fBlur,
		int& displayed_segment)
	{
		framesPerBeat *= fSpeed;
		if (framesPerBeat > 0)
		{
			int currX = (currSample - deltasample) % framesPerBeat;

			// beginning of beat -> syncronize array
			if (reset_sequence) {
				deltasample = currSample;
				reset_sequence = false;
			}
			if (deltasample < 0)
			{
				reset_sequence = true;  // can't be happening
			}

			if (currX == 0)
			{
				y = 0;
				x = 0;
			}
			else
			{
				// position in cycle
				x = (float)currX / framesPerBeat;
			}

			y = 1;

			int iSegs = vSequence.size();  // segments in sequence
			if (iSegs > 0)
			{
				int currSeg = (int)(x * iSegs);
				int segVal = vSequence[currSeg];   // value of segment (0,1)
				int segValPrev = vSequence[currSeg == 0 ? iSegs - 1 : currSeg - 1]; // value of previous segment
				if (currSeg != currSegOld) 
				{
					currSegOld = currSeg;
  					displayed_segment = currSeg;

				}
				int wSeg = framesPerBeat / iSegs;  // width (samples) of 1 segment
				int currXSeg = currX % wSeg;       // current sample pos in segment

				//fBlur = .000010f;
				dy = (float)(1 / std::max(fBlur, .001f)) * (float)currXSeg / (float)wSeg;
				if (segVal == 1) 
				{
					if (segValPrev == 1)
					{
						dy = 1;
					}
					else {
						dy = std::min(dy, 1.f);
					}
				}
				else
				{
					if (segValPrev == 0)
					{
						dy = 0;
					}
					else
					{
						dy = std::max(1 - dy, 0.f);
					}
				}
				//std::cout << "dy:" << dy << std::endl;
				y *= dy;
			}
			deltaseg++;
		}
		return y;
	}

	int Sequence::denormalizeSegments(float segs)
	{
		int retval = segs_min + segs * (segs_max - segs_min);;
		//std::cout << "Segs:"<<segs << " > "<< retval << std::endl;
		return retval;
	}

	float Sequence::normalizeSegments(int segs)
	{
		return (float)segs / (segs_max - segs_min + 1);

	}
	//int Sequence::denormalizeSequence(float sequence, int segs)
	//{
	//	int max = pow(2, segs) - 1;
	//	return (int)(sequence * max);
	//}

	std::vector<int> Sequence::sequenceToVector(uint64 iSequence, int iSegs)
	{
		std::vector<int> selection;
		uint64 val = iSequence;
		for (int i = 0; i < iSegs; i++)
		{
			uint64 v = val % 2;
			//std::cout << val << " ";
			selection.push_back(v);
			val -= v;
			val /= 2;
		}
		//std::cout << std::endl;
		return selection;
	}

	double Sequence::vectorToSequence(std::vector<int> selection)
	{
		uint64 iSequence = 0;
		for (int i = 0; i < selection.size(); i++)
		{
			if (selection[i] == 1)
			{
				iSequence += (int64)pow(2, i);
				//std::cout << iSequence << " ";
			}
		}
		//std::cout << std::endl;
		//double fSequence = iSequence / pow(10, ceil((float)selection.size() / 3));
		double fSequence = iSequence / pow(10, 5);// ceil((float)selection.size() / 3));
		//std::cout << fSequence << std::endl;
		return fSequence;
	}

	uint64 Sequence::sequenceToInt(double val, int vectorsize)
	{
		//uint64 factor = pow(10, std::ceil((float)vectorsize / 3));
		uint64 factor = pow(10, 5); //std::ceil((float)vectorsize / 3));
		uint64 retval = round(val * factor);
		return retval;
	}

	float Sequence::denormalizeSpeed(float fSpeed)
	{
		int stage = fSpeed * 8;
		float speedlist[] = { 64.,32.,16.,8.,4.,2.,1.,.5,.25 };// , .25, .125, .0625, .03125
	
		return speedlist[stage];
	}




} // namespace csse