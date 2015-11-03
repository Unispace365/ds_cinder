#ifndef _SYNC_VIDEO_PLAYER_APP_H_
#define _SYNC_VIDEO_PLAYER_APP_H_

#include <cinder/app/AppBasic.h>
#include <ds/app/app.h>

#include "app/globals.h"

#include <ds/ui/sprite/gst_video.h>
#include <ds/ui/sprite/text.h>
#include "app/video_interface/video_scrub_bar.h"

namespace ds {

class sync_video_player : public ds::App {
public:
	sync_video_player();

	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();
	virtual void		fileDrop(ci::app::FileDropEvent event);
	void				setupScrubBar();
private:
	typedef ds::App		inherited;

	// Data acquisition
	Globals				mGlobals;

	ds::ui::Sprite*		mVideoHolder;
	ds::ui::GstVideo*	mVideo;
	ds::ui::Text*		mFpsDisplay;

	bool				mVerbose;
	float				mSeekSpeed;

	//ds::ui::GstVideo*	mSelectedVideo;
	ds::ui::Video*	mSelectedVideo;
	VideoScrubBar*		mVsb;
};

} // !namespace ds

#endif // !_SYNC_VIDEO_PLAYER_APP_H_