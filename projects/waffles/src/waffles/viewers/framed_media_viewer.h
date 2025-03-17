#pragma once

#include "waffles/viewers/titled_media_viewer.h"
#include <ds/ui/media/media_interface.h>


namespace waffles {


/**
 * \class waffles::TitledMediaViewer
 *			A single media viewer with a title and a close button
 */
class FramedMediaViewer : public TitledMediaViewer {
  public:
	FramedMediaViewer(ds::ui::SpriteEngine& g, std::string eventChannel = "",
					  const std::string layoutPath = "waffles/viewer/framed_media_viewer.xml");

  protected:
	virtual void setToFullscreen(const bool immediate, const bool showController);
	virtual void onLayout() override;
	virtual void onFullscreenSet() override;
	virtual void showTitle() override;
	virtual void onMediaSet() override;
	virtual void onDetachedSet() override;

	ds::ui::MediaInterface* mMediaInterface = nullptr;

	ci::vec2 mInterfaceCheckSize;
	// virtual void hideTitle() override;
	// virtual void hideInnerSideBar() override;
};

} // namespace waffles
