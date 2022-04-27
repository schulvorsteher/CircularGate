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
	 * @param background the bitmap
	 * @param offset unused
	 */
	 //------------------------------------------------------------------------
	CSegmentsButton::CSegmentsButton(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset)
		: CControl(size, listener, tag, background)
		, offset(offset)
	{
		heightOfOneImage = size.getHeight();
		setWantsFocus(true);
	}

	//------------------------------------------------------------------------
	/**
	 * CSegmentsButton constructor.
	 * @param size the size of this view
	 * @param listener the listener
	 * @param tag the control tag
	 * @param heightOfOneImage height of one sub bitmap in background
	 * @param background the bitmap
	 * @param offset of background
	 */
	 //------------------------------------------------------------------------
	CSegmentsButton::CSegmentsButton(const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset)
		: CControl(size, listener, tag, background)
		, offset(offset)
	{
		setHeightOfOneImage(heightOfOneImage);
		setWantsFocus(true);
	}

	//------------------------------------------------------------------------
	CSegmentsButton::CSegmentsButton(const CSegmentsButton& v)
		: CControl(v)
		, offset(v.offset)
	{
		setHeightOfOneImage(v.heightOfOneImage);
		setWantsFocus(true);
	}

	//------------------------------------------------------------------------
	void CSegmentsButton::draw(CDrawContext* pContext)
	{
		CPoint where(offset.x, offset.y);

		bounceValue();

		if (value == getMax())
			where.y += heightOfOneImage;

		if (getDrawBackground())
		{
			getDrawBackground()->draw(pContext, getViewSize(), where);
		}
		setDirty(false);
	}

	//------------------------------------------------------------------------
	CMouseEventResult CSegmentsButton::onMouseDown(CPoint& where, const CButtonState& buttons)
	{
		if (!(buttons & kLButton))
			return kMouseEventNotHandled;
		beginEdit();
		return onMouseMoved(where, buttons);
	}

	//------------------------------------------------------------------------
	CMouseEventResult CSegmentsButton::onMouseCancel()
	{
		if (isEditing())
		{
			value = getMin();
			if (isDirty())
			{
				valueChanged();
				invalid();
			}
			endEdit();
		}
		return kMouseEventHandled;
	}

	//------------------------------------------------------------------------
	CMouseEventResult CSegmentsButton::onMouseUp(CPoint& where, const CButtonState& buttons)
	{
		if (isEditing())
		{
			if (value > 0.f)
				valueChanged();
			value = getMin();
			valueChanged();
			if (isDirty())
				invalid();
			endEdit();
		}
		return kMouseEventHandled;
	}

	//------------------------------------------------------------------------
	CMouseEventResult CSegmentsButton::onMouseMoved(CPoint& where, const CButtonState& buttons)
	{
		if (isEditing())
		{
			if (where.x >= getViewSize().left && where.y >= getViewSize().top &&
				where.x <= getViewSize().right && where.y <= getViewSize().bottom)
				value = getMax();
			else
				value = getMin();

			if (isDirty())
				invalid();
			return kMouseEventHandled;
		}
		return kMouseEventNotHandled;
	}

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

	//------------------------------------------------------------------------
	bool CSegmentsButton::sizeToFit()
	{
		if (getDrawBackground())
		{
			CRect vs(getViewSize());
			vs.setHeight(heightOfOneImage);
			vs.setWidth(getDrawBackground()->getWidth());
			setViewSize(vs, true);
			setMouseableArea(vs);
			return true;
		}
		return false;
	}


} // VSTGUI
