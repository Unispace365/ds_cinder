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
	FramedMediaViewer(ds::ui::SpriteEngine& g, const std::string& eventChannel = "",
					  const std::string &   layoutPath = "waffles/viewer/framed_media_viewer.xml");

	void setInterfaceLocked(bool isLocked, bool allowToggle) const override;
	bool isInterfaceLocked() const override;

  protected:
	void setToFullscreen(bool immediate, bool showController) override;
	void onLayout() override;
	void onFullscreenSet() override;
	void showTitle() override;
	void onMediaSet() override;
	void onDetachedSet() override;

	ds::ui::MediaInterface* mMediaInterface = nullptr;

	ci::vec2 mInterfaceCheckSize;
	// virtual void hideTitle() override;
	// virtual void hideInnerSideBar() override;
};

} // namespace waffles
