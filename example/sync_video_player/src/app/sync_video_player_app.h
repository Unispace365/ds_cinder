#ifndef _SYNC_VIDEO_PLAYER_APP_H_
#define _SYNC_VIDEO_PLAYER_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "app/globals.h"

#include <ds/ui/sprite/gst_video.h>
#include <ds/ui/sprite/text.h>
#include "app/video_interface/video_scrub_bar.h"

namespace ds {

class sync_video_player : public ds::App {
public:
	sync_video_player();

	virtual void				keyDown(ci::app::KeyEvent event);
	void						setupServer();
	void						update();
	virtual void				fileDrop(ci::app::FileDropEvent event);
	void						setupScrubBar();
	void						updateScrubBar(ds::ui::Video* video);
	void						loadVideo(const std::string& videoPath);
private:
	typedef ds::App				inherited;

	// Data acquisition
	Globals						mGlobals;

	ds::ui::Sprite*				mVideoHolder;
	ds::ui::GstVideo*			mVideo;
	ds::ui::Text*				mFpsDisplay;
	float						mPan;
	bool						mVerbose;
	float						mSeekSpeed;

	ds::ui::Video*				mSelectedVideo;
	VideoScrubBar*				mVsb;
	std::vector<ds::ui::Video*>	mLoadedVideos;
};

} // !namespace ds

#endif // !_SYNC_VIDEO_PLAYER_APP_H_