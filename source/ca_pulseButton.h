
#pragma once

#include "vstgui/vstgui.h"
#include "params.h"


using namespace VSTGUI;
namespace csse {

//-----------------------------------------------------------------------------
// CPulseButton Declaration
//!
/// @ingroup controls
//-----------------------------------------------------------------------------
class CPulseButton : public CControl, public IMultiBitmapControl
{
public:
	CPulseButton(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint (0, 0));

	void draw (CDrawContext*) override;

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) override;
	CMouseEventResult onMouseCancel () override;
	int32_t onKeyDown (VstKeyCode& keyCode) override;
	int32_t onKeyUp (VstKeyCode& keyCode) override;

	bool sizeToFit () override;
	void setNumSubPixmaps (int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }
	


	CLASS_METHODS(CPulseButton, CControl)
protected:
	~CPulseButton() ;
	CPoint	offset;
	CRect viewsize;

	CVSTGUITimer* timer = NULL;
	CVSTGUITimer* timer1 = NULL;
	CVSTGUITimer* timer2 = NULL;
	CVSTGUITimer* timer3 = NULL;
	void onTimer();
	int shots = 0;

	CDrawContext* context;
};


} // csse

