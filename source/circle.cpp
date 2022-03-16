#include <iostream>
#include <cstdio>

#include "circle.h"


#define PI2 (3.14159265f * 2.0f)

using namespace VSTGUI;
namespace csse
{
	// ctor
	CircleControl::CircleControl(const CRect& size) : CControl(size)
	{
	}

	// component draw method. calculate segment coords from nof segments and component area
	void CircleControl::draw(CDrawContext* pContext) {
		context = pContext;
		setTag(103);
		viewSize = getViewSize();
		drawAll();
	}

	//void CircleControl::myValueChanged()
	//{
	//	getValueNormalized();
	//	//floatToVector(value);
	//	valueChanged();
	//	
	//}

	
	void CircleControl::drawAll()
	{
		// --- setup the background rectangle
		context->setLineWidth(1);
		context->setFrameColor(CColor(200, 200, 200, 255)); // black borders
		context->setFillColor(CColor(255, 255, 255, 255)); // white background
		context->setDrawMode(kAntiAliasing);

		if (r1 == 0) 
		{
			auto viewPos = viewSize.getTopLeft();
			//CDrawContext::Transform t(*pContext, CGraphicsTransform().translate(viewPos));
			w1 = viewSize.getWidth();
			h1 = viewSize.getHeight();
			l1 = viewSize.left;
			t1 = viewSize.top;
			b1 = viewSize.bottom;
			r1 = viewSize.right;
		}


		double R = w1 / 2;
		double r = R / (1 + 1 / sin(PI2 / 2 / segs));

		for (int i = 0; i < segs; i++)
		{
			CRect seg = getSegmentRect(i, R, r);
			drawSeg(seg, i);
		}

		setDirty(false);
	}

	CRect CircleControl::getSegmentRect(int i, int R, int r)
	{
		double x = l1 + w1 / 2 + sin(PI2 / segs * i) * (w1 - 2 * r) / 2;
		double y = t1 + h1 / 2 - cos(PI2 / segs * i) * (h1 - 2 * r) / 2;
		CPoint topleft = CPoint(x - r, y - r);
		CPoint size = CPoint(2 * r, 2 * r);
		return CRect(topleft, size);
	}

	// draw 1 segment: 1 Polygon from PointList. draw selected segments in selection-color
	void CircleControl::drawSeg(CRect rect, int sel)
	{
		CGraphicsPath* drawPath = context->createGraphicsPath();
		drawPath->addEllipse(rect);
		//drawPath->closeSubpath();
		CPoint center = rect.getCenter();
		int radius = rect.getWidth() / 2;

		CColor c1;
		CColor c2;
		if (std::find(selection.begin(), selection.end(), sel) != selection.end())
		{
			c1(100, 200, 100, 50);
			c2(150, 250, 150, 255);
		}
		else
		{
			c1(00, 00, 00, 40);
			c2(15, 25, 15, 255);
		}
		CGradient* gradient = drawPath->createGradient(0.0, 1.0, c2, c1);
		context->fillRadialGradient(drawPath, *gradient, center, radius,
			CPoint(0, 0), false, nullptr);

		if (currSeg == sel)
		{
			c1(00, 00, 00, 00);
			c2(250, 250, 0, 155);
			gradient = drawPath->createGradient(0.0, 0.92, c1, c2);
			context->fillRadialGradient(drawPath, *gradient, center, radius,
				CPoint(0, 0), false, nullptr);

		}

		drawPath->forget();
		gradient->forget();

		context->drawRect(rect);

		//if (bar == sel)
		//{
		//	context->setLineWidth(2);
		//	context->setFrameColor(CColor(250, 250, 0, 255)); // borderbackground

		//	context->drawEllipse(rect, kDrawStroked);

		//	context->setLineWidth(1);
		//	context->setFrameColor(CColor(200, 200, 200, 255)); // black borders
		//}
	}


	// find the segment under mouse cursor
	int CircleControl::getSeg(CPoint& where)
	{
		if (getViewSize().pointInside(where))
		{
			double R = w1 / 2;
			double r = R / (1 + 1 / sin(PI2 / 2 / segs));

			for (int i = 0; i < segs; i++)
			{
				CRect rect = getSegmentRect(i, R, r);

				if (rect.pointInside(where))
				{
					//std::cout << "selected "<< i <<" {";
					//for (int n : selection)
					//{
					//	std::cout << n << " ";
					//}
					//std::cout << "}" << std::endl;

					return i;
				}
			}
		}
		return -1;

	}

	void CircleControl::selectNextSegment()
	{
		drawAll();
		currSeg++;
		if (currSeg > segs) currSeg = 0;
	}

	// start the selection process upon mouse down
	CMouseEventResult CircleControl::onMouseDown(CPoint& where, const CButtonState& buttons)
	{
		if (!(buttons & kLButton))
			return kMouseEventNotHandled;

		int i = getSeg(where);
		if (i >= 0)
		{
			if (std::find(selection.begin(), selection.end(), i) != selection.end())
			{
				buildSelection = -1; // remove from selection
				//selection.erase(std::remove(selection.begin(), selection.end(), i), selection.end());
			}
			else
			{
				buildSelection = 1; // add to selection 
			}

			mouseStartValue = value;
			beginEdit();
		}
		return onMouseMoved(where, buttons);
	}

	// end the selection process upon mouse up 
	CMouseEventResult CircleControl::onMouseUp(CPoint& where, const CButtonState& buttons)
	{
		if (isEditing())
		{
			value = vectorToFloat();
			if (isDirty())
				invalid();
			endEdit();
			buildSelection = 0;  // stop changing the selection 
		}
		return kMouseEventHandled;
	}

	
	// switch selection by finding 
	CMouseEventResult CircleControl::onMouseCancel()
	{
		if (isEditing())
		{
			value = mouseStartValue;
			if (isDirty())
			{
				valueChanged();
				invalid();
			}
			endEdit();
		}
		return kMouseEventHandled;
	}

	CMouseEventResult CircleControl::onMouseMoved(CPoint& where, const CButtonState& buttons)
	{
		if (isEditing())
		{
			int i = getSeg(where);
			if (i >= 0)
			{
				if (std::find(selection.begin(), selection.end(), i) != selection.end())
				{
					if (buildSelection == -1)
					{
						selection.erase(std::remove(selection.begin(), selection.end(), i), selection.end());
						setValue(vectorToFloat());
					}
				}
				else
				{
					if (buildSelection == 1)
					{
						selection.push_back(i);
						setValue(vectorToFloat());
					}
				}
				if (isDirty())
				{
					valueChanged();
					invalid();
				}
			}
		}
		return kMouseEventHandled;
	}

	double CircleControl::vectorToFloat()
	{
		double ret = 0.0;
		for (int i = 0; i < segs; i++)
		{
			if (std::find(selection.begin(), selection.end(), i) != selection.end())
			{
				ret += pow (2.0, i) ;
			}

		}
		ret /= 1000;
		//std::cout << "fSequence: ";
		//std::cout << ret << "\n";
		
		return ret;
	}

	float CircleControl::getValueNormalized()  
	{
		return vectorToFloat();
	}

	std::vector<int> CircleControl::floatToVector(float normalizedselection)
	{
		long val = 1000 * normalizedselection;
		std::vector<int> selection;
		int i = 0;
		int segs = 4;
		for (int i = 0; i < segs; i++)
		{
			int v = val % 2;
			val -= v;
			val /= 2;
			//int v = val & (2^i);
			selection.push_back(v);
		}
 		return selection;
	}

	void CircleControl::setValueNormalized(float val)
	{
		selection = floatToVector(val);
	}


	int32_t CircleControl::onKeyDown(VstKeyCode& keyCode)
	{
		return -1;
	}

	bool CircleControl::onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance,
		const CButtonState& buttons)
	{
		return true;
	}

	//float CircleControl::getSelection()
	//{
	//	return vectorToFloat();
	//}

	//void CircleControl::setSelection(float value)
	//{
	//	floatToVector(value);
	//	drawAll();
	//}

	//---send a binary message
	//void CircleControl::sendMessage()
	//{
	//	Vst::IMessage* message = this->allocateMessage();
	//	if (message)
	//	{
	//		FReleaser msgReleaser(message);
	//		message->setMessageID("BinaryMessage");

	//		uint32 size = 100;
	//		char8 data[100];
	//		memset(data, 0, size * sizeof(char));
	//		// fill my data with dummy stuff
	//		for (uint32 i = 0; i < size; i++)
	//			data[i] = i;
	//		message->getAttributes()->setBinary("MyData", data, size);
	//		controller->sendMessage(message);
	//	}
	//}


	

}//namespace csse