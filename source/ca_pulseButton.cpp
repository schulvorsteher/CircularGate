#include "vstgui/vstgui.h"
#include "ca_pulseButton.h"
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
// CPulseButton
//------------------------------------------------------------------------
/*! @class CPulseButton
Define a button with 2 states using 2 subbitmaps.
One click on it, then the second subbitmap is displayed.
When the mouse button is relaxed, the first subbitmap is framed.
*/
//------------------------------------------------------------------------
/**
 * CPulseButton constructor.
 * @param size the size of this view
 * @param listener the listener
 * @param tag the control tag
 * @param background the bitmap
 * @param offset unused
 */
//------------------------------------------------------------------------
CPulseButton::CPulseButton (const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset)
: CControl (size, listener, tag, background)
, offset (offset)
{
	heightOfOneImage = size.getHeight ();
	setWantsFocus (true);
	viewsize = size;

	if (!timer1)
	{
		timer1 = new CVSTGUITimer([this](CVSTGUITimer*) {onTimer(); }, 500, false);
	}
	if (!timer2)
	{
		timer2 = new CVSTGUITimer([this](CVSTGUITimer*) {onTimer(); }, 200, false);
	}
	if (!timer3)
	{
		timer3 = new CVSTGUITimer([this](CVSTGUITimer*) {onTimer(); }, 50, false);
	}
}

CPulseButton::~CPulseButton() 
{
	if (timer1)
	{
		timer1->forget();
		timer1 = NULL;
	}
	if (timer2)
	{
		timer2->forget();
		timer2 = NULL;
	}
	if (timer3)
	{
		timer3->forget();
		timer3 = NULL;
	}

}
//------------------------------------------------------------------------
void CPulseButton::draw (CDrawContext *pContext)
{
	CPoint where (offset.x, offset.y);
	context = pContext;
	bounceValue ();

	if (value == getMax ())
		where.y += heightOfOneImage;

	if (getDrawBackground ())
	{
		getDrawBackground ()->draw (pContext, getViewSize (), where);
	}
	setDirty (false);
}

void CPulseButton::onTimer()
{
	value = getMax();
	invalid();
	valueChanged();

	endEdit();

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	beginEdit();
	value = getMin();
	invalid();
	valueChanged();



	shots++;
	if (shots > 3) {
		timer1->stop();
		timer2->start();
		timer3->stop();
	}
	else if (shots > 10) {
		timer1->stop();
		timer2->stop();
		timer3->start();
	}

}
//------------------------------------------------------------------------
CMouseEventResult CPulseButton::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	//std::cout << "onMouseDown" << std::endl;
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;


	timer1->start();
	timer2->stop();
	timer3->stop();
	beginEdit();
	return onMouseMoved (where, buttons);
}

//------------------------------------------------------------------------
CMouseEventResult CPulseButton::onMouseCancel ()
{
	//std::cout << "onMouseCancel" << std::endl;
	if (isEditing ())
	{
		value = getMin ();
		if (isDirty ())
		{
			valueChanged ();
			invalid ();
		}
		endEdit ();
	}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CPulseButton::onMouseUp (CPoint& where, const CButtonState& buttons)
{
//	std::cout << "onMouseUp" << std::endl;
	if (isEditing ())
	{
		if (value > 0.f)
			valueChanged ();
		value = getMin ();
		valueChanged ();
		if (isDirty ())
			invalid ();
		endEdit ();
	}

	timer1->stop();
	timer2->stop();
	timer3->stop();

	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CPulseButton::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	//	std::cout << "onMouseMoved" << std::endl;
	if (isEditing ())
	{
		if (where.x >= getViewSize ().left && where.y >= getViewSize ().top  &&
			where.x <= getViewSize ().right && where.y <= getViewSize ().bottom)
			value = getMax ();
		else
			value = getMin ();
		
		if (isDirty ())
			invalid ();
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
int32_t CPulseButton::onKeyDown (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.virt == VKEY_RETURN)
	{
		if (value != getMax ())
		{
			beginEdit ();
			value = getMax ();
			invalid ();
			valueChanged ();
		}
		return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
int32_t CPulseButton::onKeyUp (VstKeyCode& keyCode)
{
	if (keyCode.modifier == 0 && keyCode.virt == VKEY_RETURN)
	{
		value = getMin ();
		invalid ();
		valueChanged ();
		endEdit ();
		return 1;
	}
	return -1;
}

//------------------------------------------------------------------------
bool CPulseButton::sizeToFit ()
{
	if (getDrawBackground ())
	{
		CRect vs (getViewSize ());
		vs.setHeight (heightOfOneImage);
		vs.setWidth (getDrawBackground ()->getWidth ());
		setViewSize (vs, true);
		setMouseableArea (vs);
		return true;
	}
	return false;
}




}// namespace csse
