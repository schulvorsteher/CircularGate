#pragma once

#include "vstgui/vstgui.h"
#include "params.h"

using namespace VSTGUI;
namespace csse {

	//-----------------------------------------------------------------------------
	// CSegmentsButton Declaration
	//! @brief another segment knob control 
	/// @ingroup controls
	//-----------------------------------------------------------------------------
	class CSegmentsButton : public CControl, public IMultiBitmapControl
	{
	public:
		CSegmentsButton(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint(0, 0));
		CSegmentsButton(const CRect& size, IControlListener* listener, int32_t tag, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint(0, 0));
		CSegmentsButton(const CSegmentsButton& button);

		// overrides
		void draw(CDrawContext* pContext) override;

		CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseCancel() override;
		int32_t onKeyDown(VstKeyCode& keyCode) override;
		int32_t onKeyUp(VstKeyCode& keyCode) override;

		bool sizeToFit() override;

		void setNumSubPixmaps(int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps(numSubPixmaps); invalid(); }

		CLASS_METHODS(CSegmentsButton, CControl)
	protected:
		~CSegmentsButton() noexcept override = default;
		//map;
		CPoint offset;

		int segs = 4;
		int currSeg = 0;
	};

} // csse
