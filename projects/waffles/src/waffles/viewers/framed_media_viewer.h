#pragma once

#include "waffles/viewers/titled_media_viewer.h"



namespace waffles {



/**
 * \class waffles::TitledMediaViewer
 *			A single media viewer with a title and a close button
 */
class FramedMediaViewer : public TitledMediaViewer {
  public:
	FramedMediaViewer(ds::ui::SpriteEngine& g, std::string eventChannel = "");

  protected:

	virtual void onLayout() override;
	virtual void onFullscreenSet() override;
	//virtual void hideTitle() override;
	//virtual void hideInnerSideBar() override;
	
};

} // namespace waffles
