#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/app/event_client.h>
#include <ds/ui/layout/smart_layout.h>

namespace waffles {

/**
 * \class ds::SettingsViewer
 *			A viewer panel that sets settings
 */
class SettingsViewer : public BaseElement {
  public:
	SettingsViewer(ds::ui::SpriteEngine& g);

  protected:
	virtual void onLayout();
	void		 updateUi();

	ds::EventClient		 mEventClient;
	ds::ui::SmartLayout* mRootLayout = nullptr;
};

} // namespace waffles
