#pragma once

#include <ds/ui/layout/smart_layout.h>

#include <ds/app/event_client.h>
#include <ds/ui/soft_keyboard/entry_field.h>

namespace waffles {

/**
 * \class ds::StateDetail
 *			Details for a saved state
 */
class StateDetail : public ds::ui::SmartLayout {
  public:
	StateDetail(ds::ui::SpriteEngine& g);

	void hideDetails();

  protected:
	void		 layout();
	virtual void onSizeChanged() override;

	ds::EventClient mEventClient;
};

} // namespace waffles
