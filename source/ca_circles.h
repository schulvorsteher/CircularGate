#pragma once

#include "vstgui/vstgui.h"
#include "params.h"

using namespace VSTGUI;
namespace csse {

	//-----------------------------------------------------------------------------
	// CCirclesKnob Declaration
	//! @brief a segment knob control 
	/// @ingroup controls
	//-----------------------------------------------------------------------------
	class CCirclesKnob : public CKnobBase, public IMultiBitmapControl
	{
	public:
		CCirclesKnob(const CRect& size, IControlListener* listener, int32_t tag, CBitmap* background, const CPoint& offset = CPoint(0, 0));
		CCirclesKnob(const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, const CPoint& offset = CPoint(0, 0));
		CCirclesKnob(const CCirclesKnob& knob);

		//-----------------------------------------------------------------------------
		/// @name CCirclesKnob Methods
		//-----------------------------------------------------------------------------
		//@{
		void setInverseBitmap(bool val) { bInverseBitmap = val; }
		bool getInverseBitmap() const { return bInverseBitmap; }
		//@}

		// overrides
		void draw(CDrawContext* pContext) override;
		bool sizeToFit() override;
		void setHeightOfOneImage(const CCoord& height) override;
		void setBackground(CBitmap* background) override;
		void setNumSubPixmaps(int32_t numSubPixmaps) override { IMultiBitmapControl::setNumSubPixmaps(numSubPixmaps); invalid(); }

		CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
		CMouseEventResult onMouseCancel() override;
		void doMouseClick();

		bool onWheel(const CPoint& where, const CMouseWheelAxis& axis, const float& distance,
			const CButtonState& buttons) override;
		int32_t onKeyDown(VstKeyCode& keyCode) override;



		CRect getSegmentRect(int i, int R, int r);
		void drawSegAction(CRect rect, int sel);
		void drawSegBackground(CRect rect, int sel, int R, int r);
		int getSeg(CPoint& where);
		void selectNextSegment();
		void adjustSelection(int segs);
		void setValueNormalized(float val);
		float getValueNormalized() const;
		//void setNofSegments(int senewNofSegmentsgs);
		int getSegs();
		void setSegs(int newSegs);
		void segValueChanged(CControl* pControl);
		static CControl* findControlForTag(CViewContainer* parent, int32_t tag, bool reverse = true);
		CDrawContext* getContext();
		void displaySegs();



		CLASS_METHODS(CCirclesKnob, CKnobBase)
	protected:
		~CCirclesKnob() noexcept override = default;
		bool	bInverseBitmap;

		CDrawContext* context;
		int w1, h1, l1, t1, b1, r1 = 0;
		std::vector<int> selection;
		int buildSelection = 0;  // 1: add to selection mode , -1 remove from selection mode
		float mouseStartValue;

		int segs = 4;
		int currSeg = 0;

		CControl* segsControl = nullptr;
		CControl* segsUpControl = nullptr;
		CControl* segsDownControl = nullptr;
	};

	//class MyController : public DelegationController, public CBaseObject
	//{
	//public:
	//	MyController(IController* baseController) : DelegationController(baseController), controlView(nullptr);
	//	~MyController();
	//	CView* verifyView(CView* view, const UIAttributes& attributes, IUIDescription* description);
	//	void valueChanged(CControl* pControl) override;

	//protected:
	//	CControl* controlView;
	//};

} // csse
