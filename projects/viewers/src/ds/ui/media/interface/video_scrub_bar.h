#pragma once
#ifndef DS_UI_MEDIA_VIDEO_INTERFACE_VIDEO_SCRUB_BAR
#define DS_UI_MEDIA_VIDEO_INTERFACE_VIDEO_SCRUB_BAR

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {
class GstVideo;

/**
* \class VideoScrubBar
*			Seeking / playback info for  a video
*/
class VideoScrubBar : public ds::ui::Sprite  {
public:
	VideoScrubBar(ds::ui::SpriteEngine& eng, const float heighty, const float buttHeight, const ci::Color interfaceColor);

	void					linkVideo(ds::ui::GstVideo* linkedVideo);
	virtual void			onUpdateServer(const ds::UpdateParams& p) override;
	void					layout();

protected:
	virtual void			onSizeChanged();

	ds::ui::GstVideo*		mLinkedVideo;
	ds::ui::Sprite*			mBacker;
	ds::ui::Sprite*			mProgress;


};

} // namespace ui
} // namespace ds

#endif
