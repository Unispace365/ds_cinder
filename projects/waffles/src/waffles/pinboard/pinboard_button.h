#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace waffles {

// This is a button that gets linked to an item. When tapped, toggles the item being on the pinboard. Updates the status
// of the icon automatically
class PinboardButton : public ds::ui::SmartLayout {
  public:
	PinboardButton(ds::ui::SpriteEngine& g, std::string theLayoutFile);

	static bool itemIsOnPinboard(ds::ui::SpriteEngine& eng, ds::model::ContentModelRef model);
	void		setPinnedStatus();
	void		toggleOnPinboard();

	virtual void onUpdateServer(const ds::UpdateParams& p);

  private:
  	bool mAmSaving = false;
};

} // namespace waffles
