#pragma once
#ifndef DS_UI_MEDIA_VIEWER_VIDEO_INTERFACE
#define DS_UI_MEDIA_VIEWER_VIDEO_INTERFACE

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {

class GstVideo;
class ImageButton;
class VideoScrubBar;
class VideoVolumeControl;

/**
* \class ds::ui::VideoInterface
*			Implements play/pause, scrub bar, volume control
*/
class VideoInterface : public MediaInterface  {
public:
	VideoInterface(ds::ui::SpriteEngine& eng, const ci::Vec2f& interfaceSize, const float buttonHeight, const ci::Color buttonColor, const ci::Color backgroundColor);

	void						linkVideo(ds::ui::GstVideo* linkedVideo);

	virtual void				updateServer(const ds::UpdateParams& p);
protected:

	virtual void				onLayout();

	ds::ui::GstVideo*			mLinkedVideo;

	ds::ui::ImageButton*		mPlayButton;
	ds::ui::ImageButton*		mPauseButton;

	VideoScrubBar*				mScrubBar;
	VideoVolumeControl*			mVolumeControl;

};

} // namespace ui
} // namespace ds

#endif
