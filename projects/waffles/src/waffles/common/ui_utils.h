#pragma once

#include <ds/ui/media/media_interface.h>

namespace ds::ui {
class SmartLayout;
}

namespace waffles {

class ContentUtils : ds::ui::Sprite {
  public:
	ContentUtils(ds::ui::SpriteEngine& g);
	// void	getThumbnailForContent(ds::model::ContentModelRef inputModel, std::string& outputString, bool& valid);
	static ContentUtils* getDefault(ds::ui::SpriteEngine& g);
	static void			 configureListItem(ds::ui::SpriteEngine& engine, ds::ui::SmartLayout* item,
										   const ci::vec2& size = ci::vec2(-1));
	static bool handleListItemTap(ds::ui::SpriteEngine& engine, ds::ui::SmartLayout* item, const std::string& channel,
								  const ci::vec3& pos = ci::vec3(-1));
	[[deprecated]] static void setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey);


	bool		isFolder(ds::model::ContentModelRef model) const;
	bool		isMedia(ds::model::ContentModelRef model) const;
	bool		isPresentation(ds::model::ContentModelRef model) const;
	bool		isAmbientPlaylist(ds::model::ContentModelRef model) const;
	std::string getMediaPropertyKey(ds::model::ContentModelRef model) const;
};

} // namespace waffles
