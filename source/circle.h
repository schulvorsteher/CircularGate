#pragma once

#include "vstgui/vstgui.h"
#include "pluginterfaces/vst/ivstmessage.h"


using namespace VSTGUI;
namespace csse
{

	class CircleControl : public CControl, protected CMouseWheelEditingSupport
	{
	public:
		CircleControl(const CRect& size);
		void draw(CDrawContext* pContext) override;
		void drawAll();
		CLASS_METHODS(CircleControl, CControl)

		//int selectedSeg = -1;

		// overrides
		CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseCancel() override;
		void doMouseClick();

		bool onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance,
			const CButtonState& buttons) override;
		int32_t onKeyDown(VstKeyCode& keyCode) override;

		int getSeg(CPoint& where);

		void sendMessage();

		int kSequencId;

		double vectorToFloat();
		std::vector<int> floatToVector(float normalizedselection);

		CRect viewSize;

		//float getSelection();
		//void setSelection(float value);
		virtual float getValueNormalized() ;
		virtual void setValueNormalized(float val) ;

	protected:
		std::vector<int> selection;
		CDrawContext* context;
		int segs = 4;
		int currSeg = 0;
		CRect getSegmentRect(int i, int R, int r);
		void drawSeg(CRect, int sel);
		int w1,h1, l1, t1, b1, r1 = 0;

		int buildSelection = 0;  // 1: add to selection mode , -1 remove from selection mode
		
		float mouseStartValue;

		bool notify(Steinberg::Vst::IMessage* message);
		void selectNextSegment();
		//void myValueChanged();

	};
}
