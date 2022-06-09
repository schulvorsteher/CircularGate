#include "vstgui/vstgui.h"
#include "ca_segments.h"
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
	// CSegmentsButton
	//------------------------------------------------------------------------
	/*! @class CSegmentsButton
	*/
	//------------------------------------------------------------------------
	/**
	 * CSegmentsButton constructor.
	 * @param size the size of this view
	 * @param listener the listener
	 * @param tag the control tag
	 * @param background the background bitmap
	 * @param offset unused
	 */
	 //------------------------------------------------------------------------
	CSegmentsButton::CSegmentsButton(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset)
		: CKnobBase(size, listener, tag, background)
		, bInverseBitmap(false)
	{
		heightOfOneImage = size.getHeight();
		setNumSubPixmaps(background ? (int32_t)(background->getHeight() / heightOfOneImage) : 0);
		inset = 0;
		

	}

	//------------------------------------------------------------------------
	/**
	 * CSegmentsButton constructor.
	 * @param size the size of this view
	 * @param listener the listener
	 * @param tag the control tag
	 * @param subPixmaps number of sub bitmaps in background
	 * @param heightOfOneImage the height of one sub bitmap
	 * @param background the background bitmap
	 * @param offset unused
	 */
	 //------------------------------------------------------------------------
	CSegmentsButton::CSegmentsButton(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
		: CKnobBase(size, listener, tag, background)
		, bInverseBitmap(false)
	{
		setNumSubPixmaps(subPixmaps);
		setHeightOfOneImage(heightOfOneImage);
		inset = 0;

	}

	//------------------------------------------------------------------------
	CSegmentsButton::CSegmentsButton(const CSegmentsButton& v)
		: CKnobBase(v)
		, bInverseBitmap(v.bInverseBitmap)
	{
		setNumSubPixmaps(v.subPixmaps);
		setHeightOfOneImage(v.heightOfOneImage);
	}

	//-----------------------------------------------------------------------------------------------
	bool CSegmentsButton::sizeToFit()
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
	//void CSegmentsButton::setHeightOfOneImage(const CCoord& height)
	//{
	//	IMultiBitmapControl::setHeightOfOneImage(height);
	//	if (getDrawBackground() && heightOfOneImage > 0)
	//		setNumSubPixmaps((int32_t)(getDrawBackground()->getHeight() / heightOfOneImage));
	//}

	//-----------------------------------------------------------------------------------------------
	//void CSegmentsButton::setBackground(CBitmap* background)
	//{
	//	CKnobBase::setBackground(background);
	//	if (heightOfOneImage == 0)
	//		heightOfOneImage = getViewSize().getHeight();
	//	if (background && heightOfOneImage > 0)
	//		setNumSubPixmaps((int32_t)(background->getHeight() / heightOfOneImage));
	//}

	//------------------------------------------------------------------------
	void CSegmentsButton::draw(CDrawContext* pContext)
	{

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

		CRect viewSize = getViewSize();
		auto viewPos = getViewSize().getTopLeft();
		int w1 = viewSize.getWidth();
		int h1 = viewSize.getHeight();
		int l1 = viewSize.left;
		int t1 = viewSize.top;
		int b1 = viewSize.bottom;
		int r1 = viewSize.right;

		//buttonState = 0;

		pContext->setFrameColor(CColor(100, 150, 0, 255)); // black borders
		CGraphicsPath* drawPath = pContext->createGraphicsPath();
		CColor c1;
		CColor c2;
		CColor c3;
		CColor c4;
		CColor cFont;

		c1(100, 200, 100, 50);  // state on dark
		c2(150, 250, 150, 255); // state on light
		c3(100, 120, 100, 255); // state off light
		c4(60,120, 60, 255); // state off dark

		cFont(0, 0, 0, 255); 
		int radius = 100;
		CPoint center(l1 + w1 / 2, t1 + h1 / 2);
		CGradient* gradientLight = drawPath->createGradient(0.0, 1.0, c2, c1);
		CGradient* gradientDark = drawPath->createGradient(0.0, 1.0, c4, c3);
		

		// arrow up
		drawPath->beginSubpath(l1, t1 + h1 / 3);
		drawPath->addLine(l1 + w1, t1 + h1 / 3);
		drawPath->addLine(l1 + w1 / 2, t1);
		drawPath->closeSubpath();
		if (buttonState == 0 || buttonState == 2)
			pContext->fillRadialGradient(drawPath, *gradientDark, center, radius, CPoint(0, 0), true, nullptr);
		else if (buttonState == 1)
			pContext->fillRadialGradient(drawPath, *gradientLight, center, radius, CPoint(0, 0), true, nullptr);
		pContext->drawGraphicsPath(drawPath, CDrawContext::kPathStroked, nullptr);

		// arrow down
		drawPath = pContext->createGraphicsPath();
		drawPath->beginSubpath(l1, t1 + 2 * h1 / 3);
		drawPath->addLine(l1 + w1, t1 + 2 * h1 / 3);
		drawPath->addLine(l1 + w1 / 2, t1 + h1);
		drawPath->closeSubpath();
		if (buttonState == 0 || buttonState == 1)
			pContext->fillRadialGradient(drawPath, *gradientDark, center, radius, CPoint(0, 0), true, nullptr);
		else if (buttonState == 2)
			pContext->fillRadialGradient(drawPath, *gradientLight, center, radius, CPoint(0, 0), true, nullptr);
		pContext->drawGraphicsPath(drawPath, CDrawContext::kPathStroked, nullptr);;

		drawPath->forget();
		gradientLight->forget();
		gradientDark->forget();
	
			
		setDirty(false);
	}

	// find the segment under mouse cursor
	int CSegmentsButton::getButton(CPoint& where)
	{
		int bs = 0;
		if (getViewSize().pointInside(where))
		{
			int h1 = getViewSize().getHeight();
			int t1 = getViewSize().top;

			if (where.y - t1 < h1 / 3)
				bs = 1;
			else if (where.y - t1 > 2 * h1 / 3)
				bs = 2;

			//std::cout << "buttonstate:" << bs<< std::endl;


		}
		return bs;
	}


	//------------------------------------------------------------------------
	CMouseEventResult CSegmentsButton::onMouseDown(CPoint& where, const CButtonState& buttons)
	{
		if (!(buttons & kLButton))
			return kMouseEventNotHandled;

		buttonState = getButton(where);
		beginEdit();
		return onMouseMoved(where, buttons);
	}

	// switch selection by finding 
	CMouseEventResult CSegmentsButton::onMouseCancel()
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
			buttonState = 0;
		}
		return kMouseEventHandled;
	}

	// end the selection process upon mouse up 
	CMouseEventResult CSegmentsButton::onMouseUp(CPoint& where, const CButtonState& buttons)
	{
		if (isEditing())
		{
			if (value > 0.f)
				valueChanged();
			
			if (buttonState == 1) value += (float)1 / 32;
			else if (buttonState == 2) value -= (float)1 / 32;

			valueChanged();
			if (isDirty())
				invalid();
			endEdit();
			buttonState = 0;
		}
		return kMouseEventHandled;
	}

	//------------------------------------------------------------------------
	CMouseEventResult CSegmentsButton::onMouseMoved(CPoint& where, const CButtonState& buttons)
	{
		if (isEditing())
		{
			buttonState = getButton(where);
			valueChanged();
			if (isDirty())
				invalid();
			endEdit();
			return kMouseEventHandled;
		}
		return kMouseEventHandled;
	}

	//bool CSegmentsButton::onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance,
	//	const CButtonState& buttons)
	//{
	//	return true;
	//}

	//------------------------------------------------------------------------
	int32_t CSegmentsButton::onKeyDown(VstKeyCode& keyCode)
	{
		if (keyCode.modifier == 0 && keyCode.virt == VKEY_RETURN)
		{
			if (value != getMax())
			{
				beginEdit();
				value = getMax();
				invalid();
				valueChanged();
			}
			return 1;
		}
		return -1;
	}

	//------------------------------------------------------------------------
	int32_t CSegmentsButton::onKeyUp(VstKeyCode& keyCode)
	{
		if (keyCode.modifier == 0 && keyCode.virt == VKEY_RETURN)
		{
			value = getMin();
			invalid();
			valueChanged();
			endEdit();
			return 1;
		}
		return -1;
	}


}// namespace csse
