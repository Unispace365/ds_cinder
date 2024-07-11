#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/app/event_client.h>
#include <ds/ui/layout/smart_layout.h>

namespace waffles {
class StateDetail;

/**
 * \class ds::StateViewer
 *			A viewer that shows what saved states are available
 */
class StateViewer : public BaseElement {
  public:
	StateViewer(ds::ui::SpriteEngine& g);

  protected:
	void		 showDetails(ds::model::ContentModelRef info);
	virtual void onLayout();
	void		 setData();

	std::map<int, ds::model::ContentModelRef> mInfoMap;
	ds::EventClient							  mEventClient;
	ds::ui::SmartLayout*					  mRootLayout;
	StateDetail*							  mStateDetail;
};

} // namespace waffles
