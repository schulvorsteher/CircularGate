#include "vstgui/vstgui.h"

using namespace VSTGUI;
using namespace VSTGUI::Animation;

class CircleView : public CView
{
public:
	CircleView(const CRect& r) : CView(r) {
		setAlphaValue(0.5f);
	}

	CMouseEventResult onMouseEntered(CPoint& where, const CButtonState& buttons)
	{
		// this adds an animation which takes 200ms to make a linear alpha fade from the current value to 1
		addAnimation("AlphaValueAnimation", new AlphaValueAnimation(1.0f), new LinearTimingFunction(200));
		return kMouseEventHandled;
	}

	CMouseEventResult onMouseExited(CPoint& where, const CButtonState& buttons)
	{
		// this adds an animation which takes 200ms to make a linear alpha fade from the current value to 0.5
		addAnimation("AlphaValueAnimation", new AlphaValueAnimation(0.5f), new LinearTimingFunction(200));
		return kMouseEventHandled;
	}

	void draw(CDrawContext* context)
	{

	}
};

