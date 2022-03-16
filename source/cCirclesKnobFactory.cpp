#pragma once

#include "vstgui/vstgui.h"
#include "vstgui/vstgui_uidescription.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"

// replace this include by the header file of your new view
#include "ca_circles.h"

using namespace VSTGUI;
namespace csse {

	class CCirclesKnobFactory : ViewCreatorAdapter
	{
	public:
		// register this class with the view factory
        CCirclesKnobFactory()
		{
			UIViewFactory::registerViewCreator(*this);
		}

		// return an uniquie name here
		IdStringPtr getViewName() const override 
		{
			return "CCirclesKnob";
		}

		// return the name here from where your custom view inherites
		// Your view automatically supports the attributes from it.
		IdStringPtr getBaseViewName() const override
		{
			return UIViewCreator::kCControl;
		}

		// create your view here.
		// Note you don't need to apply attributes here as
		// the apply method will be called with this new view
		CView* create(const UIAttributes& attributes, const IUIDescription* description) const override
		{
			CRect size(CPoint(0, 0), CPoint(200, 200));
            auto* knob = new CCirclesKnob(size, nullptr, 0, nullptr);
            IController* c = description->getController();
            //c->setMyControllerView(knob);
            return knob;
		}

	};
	// create a static instance so the it registers itself with the view factoty
	CCirclesKnobFactory __gCCirclesKnobFactory;

} // namespace csse