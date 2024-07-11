#pragma once

#include <ds/ui/media/media_interface.h>

namespace ds::ui {
	class SmartLayout;
}

namespace waffles {

// void	getThumbnailForContent(ds::model::ContentModelRef inputModel, std::string& outputString, bool& valid);
void configureListItem(ds::ui::SpriteEngine& engine, ds::ui::SmartLayout* item, ci::vec2 size = ci::vec2(-1));
bool handleListItemTap(ds::ui::SpriteEngine& engine, ds::ui::SmartLayout* item, ci::vec3 pos = ci::vec3(-1));
void setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey);

} // namespace waffles
