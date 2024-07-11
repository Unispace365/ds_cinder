#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/ui/layout/smart_layout.h>

namespace waffles {

/**
 * \class ds::ErrorViewer
 *			Displays a brief error message to the user
 */
class ErrorViewer : public BaseElement {
  public:
	ErrorViewer(ds::ui::SpriteEngine& g);

	virtual void onMediaSet() override;

  protected:
	virtual void onLayout();

	ds::ui::SmartLayout* mPrimaryLayout;
};

} // namespace waffles
