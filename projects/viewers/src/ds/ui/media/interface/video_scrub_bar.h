#pragma once

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui {
class GstVideo;
class IPdf;
class YouTubeWeb;

/**
* \class VideoScrubBar
*			Seeking / playback info for a video *And now PDFs* *And now YouTubes*
*/
class VideoScrubBar : public ds::ui::Sprite  {
public:
	VideoScrubBar(ds::ui::SpriteEngine& eng, const float heighty, const float buttHeight, const ci::Color interfaceColor);

	void					linkVideo(ds::ui::GstVideo* linkedVideo);
	void					linkPdf(ds::ui::IPdf* linkedPdf);
	void					linkYouTube(ds::ui::YouTubeWeb* linkedYouTube);
	virtual void			onUpdateServer(const ds::UpdateParams& p) override;
	void					layout();

	ds::ui::Sprite*			getBacker();
	ds::ui::Sprite*			getProgress();

	void					addNub(ds::ui::Sprite* nub);
	
protected:
	virtual void			onSizeChanged();
	void					setProgressPercent(const float theProgress);

	ds::ui::GstVideo*		mLinkedVideo;
	ds::ui::IPdf*			mLinkedPdf;
	ds::ui::YouTubeWeb*		mLinkedYouTube;
	ds::ui::Sprite*			mBacker;
	ds::ui::Sprite*			mProgress;

	ds::ui::Sprite*			mNub = nullptr;
};

} // namespace ui
} // namespace ds

