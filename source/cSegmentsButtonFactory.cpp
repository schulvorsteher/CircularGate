#pragma once

#include "vstgui/vstgui.h"
#include "vstgui/vstgui_uidescription.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"

// replace this include by the header file of your new view
#include "ca_segments.h"

using namespace VSTGUI;
namespace csse {

	class CSegmentsButtonFactory : ViewCreatorAdapter
	{
	public:
		// register this class with the view factory
		CSegmentsButtonFactory()
		{
			UIViewFactory::registerViewCreator(*this);
		}

		// return an uniquie name here
		IdStringPtr getViewName() const override 
		{
			return "CSegmentsButton";
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
			CRect size(CPoint(0, 0), CPoint(40,100));
            auto* button = new CSegmentsButton(size, nullptr, 0, nullptr);
            IController* c = description->getController();
            //c->setMyControllerView(knob);
            return button;
		}

	};
	// create a static instance so the it registers itself with the view factoty
	CSegmentsButtonFactory __gCSegmentsButtonFactory;

} // namespace csse