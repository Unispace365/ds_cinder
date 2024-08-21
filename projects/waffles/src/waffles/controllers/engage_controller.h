#pragma once

#include <ds/app/event_client.h>
#include <ds/ui/sprite/sprite.h>

namespace waffles {

/**
 * \class hpi::EngageController
 *			The background layer for interactive playlists and templates
 */
/*
class EngageController : public ds::ui::Sprite {
  public:
	EngageController(ds::ui::SpriteEngine& eng);

  private:
	void setDefaultPresentation();
	void setPresentation(ds::model::ContentModelRef thePlaylist);
	bool setSlide(ds::model::ContentModelRef theSlide);
	void setData();

	void nextItem();
	void prevItem();
	void gotoItem(int index);

	void startEngage();
	void endEngage();

	ds::EventClient mEventClient;

	float mInteractiveDuration = 5.f;

	std::string mPlaylistUid;
	std::string mSlideUid;
	int			mPlaylistIdx = -1;

	ds::model::ContentModelRef mPlaylist;
	ds::model::ContentModelRef mSlide;

	bool mAmEngaged = false;
};
*/
} // namespace waffles
