#pragma once

#include "vstgui/vstgui.h"
#include "params.h"

using namespace VSTGUI;
namespace csse {

	//-----------------------------------------------------------------------------
	// CSegmentsButton Declaration
	//! @brief a segment knob control 
	/// @ingroup controls
	//-----------------------------------------------------------------------------
	class CSegmentsButton : public CKnobBase, public IMultiBitmapControl
	{
	public:
		CSegmentsButton(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint(0, 0));
		CSegmentsButton(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint(0, 0));
		CSegmentsButton(const CSegmentsButton& knob);


		// overrides
		void draw(CDrawContext* pContext) override;

		CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseCancel() override;
		int32_t onKeyDown(VstKeyCode& keyCode) override;
		int32_t onKeyUp(VstKeyCode& keyCode) override;

		bool sizeToFit() override;
		int getButton(CPoint& where);

		void setNumSubPixmaps(int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps(numSubPixmaps); invalid(); }


		CLASS_METHODS(CSegmentsButton, CKnobBase)
	protected:
		~CSegmentsButton() noexcept override = default;
		bool	bInverseBitmap;

		int buildSelection = 0;  // 1: add to selection mode , -1 remove from selection mode
		float mouseStartValue;

		int segs = 4;
		int segsold = 0;
		int currSeg = 0;
		int buttonState = 0;

	};


} // csse
