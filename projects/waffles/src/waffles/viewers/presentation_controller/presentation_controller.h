#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/ui/layout/smart_layout.h>

namespace waffles {

/**
 * \class ds::PresentationController
 *			A viewer panel that controls presentations
 */
class PresentationController : public BaseElement {
  public:
	PresentationController(ds::ui::SpriteEngine& g);

  protected:
	virtual void onLayout();
	virtual void onCreationArgsSet();

	void updateUi();
	void updatePins();
	void toggleThumbs();
	void togglePins();
	void reconfigureSize();

	ds::EventClient		 mEventClient;
	ds::ui::SmartLayout* mRootLayout;

	ds::model::ContentModelRef mCurrentSlide;

	std::map<std::string, ds::Resource> mThumbMap;
	bool								mShowingThumbs;

	std::map<std::string, ds::Resource> mPinMap;
	bool								mShowingPins;

	bool mResetButtonEnabled = true;
};

} // namespace waffles
