#include "vstgui/vstgui.h"
#include "ca_circles.h"
#include "ca_sequence.h"
#include <cmath>
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "vstgui/vstgui_uidescription.h"
//#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

using namespace VSTGUI;
namespace csse {


	//------------------------------------------------------------------------
	// CCirclesKnob
	//------------------------------------------------------------------------
	/*! @class CCirclesKnob
	*/
	//------------------------------------------------------------------------
	/**
	 * CCirclesKnob constructor.
	 * @param size the size of this view
	 * @param listener the listener
	 * @param tag the control tag
	 * @param background the background bitmap
	 * @param offset unused
	 */
	 //------------------------------------------------------------------------
	CCirclesKnob::CCirclesKnob(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset)
		: CKnobBase(size, listener, tag, background)
		, bInverseBitmap(false)
	{
		heightOfOneImage = size.getHeight();
		setNumSubPixmaps(background ? (int32_t)(background->getHeight() / heightOfOneImage) : 0);
		inset = 0;
		

	}

	//------------------------------------------------------------------------
	/**
	 * CCirclesKnob constructor.
	 * @param size the size of this view
	 * @param listener the listener
	 * @param tag the control tag
	 * @param subPixmaps number of sub bitmaps in background
	 * @param heightOfOneImage the height of one sub bitmap
	 * @param background the background bitmap
	 * @param offset unused
	 */
	 //------------------------------------------------------------------------
	CCirclesKnob::CCirclesKnob(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
		: CKnobBase(size, listener, tag, background)
		, bInverseBitmap(false)
	{
		setNumSubPixmaps(subPixmaps);
		setHeightOfOneImage(heightOfOneImage);
		inset = 0;

	}

	//------------------------------------------------------------------------
	CCirclesKnob::CCirclesKnob(const CCirclesKnob& v)
		: CKnobBase(v)
		, bInverseBitmap(v.bInverseBitmap)
	{
		setNumSubPixmaps(v.subPixmaps);
		setHeightOfOneImage(v.heightOfOneImage);
	}

	//-----------------------------------------------------------------------------------------------
	bool CCirclesKnob::sizeToFit()
	{
		if (getDrawBackground())
		{
			CRect vs(getViewSize());
			vs.setWidth(getDrawBackground()->getWidth());
			vs.setHeight(getHeightOfOneImage());
			setViewSize(vs);
			setMouseableArea(vs);
			return true;
		}
		return false;
	}

	//-----------------------------------------------------------------------------------------------
	void CCirclesKnob::setHeightOfOneImage(const CCoord& height)
	{
		IMultiBitmapControl::setHeightOfOneImage(height);
		if (getDrawBackground() && heightOfOneImage > 0)
			setNumSubPixmaps((int32_t)(getDrawBackground()->getHeight() / heightOfOneImage));
	}

	//-----------------------------------------------------------------------------------------------
	void CCirclesKnob::setBackground(CBitmap* background)
	{
		CKnobBase::setBackground(background);
		if (heightOfOneImage == 0)
			heightOfOneImage = getViewSize().getHeight();
		if (background && heightOfOneImage > 0)
			setNumSubPixmaps((int32_t)(background->getHeight() / heightOfOneImage));
	}

	//------------------------------------------------------------------------
	void CCirclesKnob::draw(CDrawContext* pContext)
	{
		CControl* segControl = findControlForTag(getParentView()->asViewContainer(), kSegsId, false);
		if (segControl == nullptr)
		{
			segControl = findControlForTag(getFrame(), kSegsId, true);
		}
		if (segControl )
		{
			segValueChanged(segControl);
			adjustSelection(getSegs());
		}

		if (getDrawBackground())
		{
			CPoint where(0, 0);
			float val = getValueNormalized();
			if (val >= 0.f && heightOfOneImage > 0.)
			{
				CCoord tmp = heightOfOneImage * (getNumSubPixmaps() - 1);
				if (bInverseBitmap)
					where.y = floor((1. - val) * tmp);
				else
					where.y = floor(val * tmp);
				where.y -= (int32_t)where.y % (int32_t)heightOfOneImage;
			}

			getDrawBackground()->draw(pContext, getViewSize(), where);
		}


		//pContext->setLineWidth(1);
		//pContext->setFrameColor(CColor(200, 200, 200, 255)); // black borders
		//pContext->setFillColor(CColor(255, 255, 255, 255)); // white background
		pContext->setDrawMode(kAntiAliasing);

		CRect viewSize = getViewSize();
		//viewSize.extend(-4, -4);
		auto viewPos = getViewSize().getTopLeft();
		w1 = viewSize.getWidth();
		h1 = viewSize.getHeight();
		l1 = viewSize.left;
		t1 = viewSize.top;
		b1 = viewSize.bottom;
		r1 = viewSize.right;

		context = pContext;

		double R = w1 / 2;
		double r = R / (1 + 1 / sin(Constants::double_pi / 2 / getSegs()));

		if (tag == kCurrSegmentId)
		{
			float v = getValueNormalized();
			float v1 = getValue();

			int i = v * 100;// getSegs();
			//std::cout << "ca_circles 163: currSegment: ";
			//std::cout << i << std::endl;
			
			CRect seg = getSegmentRect(i, R, r);
			drawSegBackground(seg, i, R, r);
		}
		if (tag == kSequenceId)
		{
			for (int i = 0; i < getSegs(); i++)
			{
				CRect seg = getSegmentRect(i, R, r);
				drawSegAction(seg, i);
			}
			setDirty(false);
		}

	}

	CRect CCirclesKnob::getSegmentRect(int i, int R, int r)
	{
		double x = l1 + w1 / 2 + sin(Constants::double_pi / getSegs() * i) * (w1 - 2 * r) / 2;
		double y = t1 + h1 / 2 - cos(Constants::double_pi / getSegs() * i) * (h1 - 2 * r) / 2;
		CPoint topleft = CPoint(x - r, y - r);
		CPoint size = CPoint(2 * r, 2 * r);
		return CRect(topleft, size);
	}

	void CCirclesKnob::drawSegBackground(CRect rect, int sel, int R, int r)
	{
		if (sel < 100)
		{
			CPoint center = rect.getCenter();
			int radius = rect.getWidth() / 2;
			CGraphicsPath* drawPath = context->createGraphicsPath();
			drawPath->addEllipse(rect);// .extend(CPoint(5, 5)));

			CColor c1(100, 100, 00, 100);
			CColor c2(250, 250, 0, 155);

			CGradient* gradient = drawPath->createGradient(0.5, 1.0, c1, c2);
			context->fillRadialGradient(drawPath, *gradient, center, radius,
				CPoint(0,0), false, nullptr);
			gradient->forget();
			drawPath->forget();
		}
		else
		{
			//rect.extend(CPoint(1, 1));
			context->setFrameColor(CColor(200, 200, 0, 255)); // black borders
			context->drawEllipse(rect, kDrawStroked);
		}
	}

	void CCirclesKnob::drawSegAction(CRect rect, int sel)
	{
		CGraphicsPath* drawPath = context->createGraphicsPath();
		drawPath->addEllipse(rect);
		CPoint center = rect.getCenter();
		int radius = rect.getWidth() / 2;

		CColor c1;
		CColor c2;
		CColor cFont;
		if (selection.size() > 0 && selection[sel] == 1)
			//if (std::find(selection.begin(), selection.end(), sel) != selection.end())
		{
			c1(100, 200, 100, 50);
			c2(150, 250, 150, 255);
			cFont(0, 0, 0, 255);
		}
		else
		{
			c1(00, 00, 00, 40);
			c2(15, 25, 15, 255);
			cFont(255,255,255,255);
		}
		CGradient* gradient = drawPath->createGradient(0.0, 1.0, c2, c1);
		context->fillRadialGradient(drawPath, *gradient, center, radius,
			CPoint(0, 0), false, nullptr);
	
		//context->setFontColor(cFont);
		//context->drawString((UTF8String)std::to_string(sel + 1), rect, kCenterText, true);
	}


	// find the segment under mouse cursor
	int CCirclesKnob::getSeg(CPoint& where)
	{
		if (getViewSize().pointInside(where))
		{
			double R = w1 / 2;
			double r = R / (1 + 1 / sin(Constants::double_pi / 2 / getSegs()));

			for (int i = 0; i < getSegs(); i++)
			{
				CRect rect = getSegmentRect(i, R, r);

				if (rect.pointInside(where))
				{
					return i;
				}
			}
		}
		return -1;

	}

	void CCirclesKnob::selectNextSegment()
	{
		//drawAll();
		currSeg++;
		if (currSeg > getSegs()) currSeg = 0;
	}

	CDrawContext* CCirclesKnob::getContext()
	{
		return context;
	}


	// start the selection process upon mouse down
	CMouseEventResult CCirclesKnob::onMouseDown(CPoint& where, const CButtonState& buttons)
	{
		if (!(buttons & kLButton))
			return kMouseEventNotHandled;

		int i = getSeg(where);
		if (i >= 0)
		{
			adjustSelection(getSegs());
			if (selection.size() > 0 && selection[i] == 1)
			{
				buildSelection = -1; // remove from selection
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
	CMouseEventResult CCirclesKnob::onMouseUp(CPoint& where, const CButtonState& buttons)
	{
		if (isEditing())
		{
			value = Sequence::vectorToSequence(selection);
			if (isDirty())
				invalid();
			endEdit();
			buildSelection = 0;  // stop changing the selection 
		}
		return kMouseEventHandled;
	}


	// switch selection by finding 
	CMouseEventResult CCirclesKnob::onMouseCancel()
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

	CMouseEventResult CCirclesKnob::onMouseMoved(CPoint& where, const CButtonState& buttons)
	{
		if (isEditing())
		{
			int i = getSeg(where);
			if (i >= 0)
			{
				if (i < selection.size() && selection[i] == 1)
				{
					if (buildSelection == -1)
					{
						selection[i] = 0;
						setValue(Sequence::vectorToSequence(selection));
					}
				}
				else
				{
					if (buildSelection == 1)
					{
						selection[i] = 1;
						setValue(Sequence::vectorToSequence(selection));
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

	bool CCirclesKnob::onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance,
		const CButtonState& buttons)
	{
		return true;
	}

	int32_t CCirclesKnob::onKeyDown(VstKeyCode& keyCode)
	{
		return -1;
	}

	void CCirclesKnob::adjustSelection(int segs)
	{
		//segs = segs_max;
		while (selection.size() < segs)
		{
			selection.push_back(0);
		}
		//while (selection.size() > segs)
		//{
		//	selection.pop_back();
		//}
	}

	void CCirclesKnob::setValueNormalized(float val)
	{
		if (val > 1.f)
			val = 1.f;
		else if (val < 0.f)
			val = 0.f;
		setValue(getRange() * val + getMin());

		int64 iSequence = Sequence::sequenceToInt(val, getSegs());
		selection = Sequence::sequenceToVector(iSequence, getSegs());
	}

	//------------------------------------------------------------------------
	float CCirclesKnob::getValueNormalized() const
	{
		auto range = getRange();
		if (range == 0.f)
			return 0.f;
		return (value - getMin()) / range;
	}

	int CCirclesKnob::getSegs()
	{
		if (segs < segs_min) segs = segs_min;
		if (segs > segs_max) segs = segs_max;
		return segs;
	}

	void CCirclesKnob::setSegs(int newSegs)
	{
		segs = newSegs;
	}


	CControl* CCirclesKnob::findControlForTag(CViewContainer* parent, int32_t tag, bool reverse)
	{
		CControl* result = nullptr;
		ViewIterator it(parent);
		while (*it)
		{
			CView* view = *it;
			auto* control = dynamic_cast<CControl*> (view);
			if (control)
			{
				if (control->getTag() == tag)
					result = control;
			}
			else if (reverse)
			{
				if (auto container = view->asViewContainer())
					result = findControlForTag(container, tag);
			}
			if (result)
				break;
			++it;
		}
		if (result == nullptr && !reverse && parent->getParentView())
			return findControlForTag(parent->getParentView()->asViewContainer(), tag, reverse);
		return result;
	}

	void CCirclesKnob::segValueChanged(CControl* pControl)
	{
		auto norm = pControl->getValueNormalized();
		int segs = int(pControl->getValue());
		if (segs != segsold) {
			//std::cout << "ca_circles 460: segments: ";
			//std::cout << segs << std::endl;
			setSegs(segs);
			segsold = segs;
		}
	}


}// namespace csse
