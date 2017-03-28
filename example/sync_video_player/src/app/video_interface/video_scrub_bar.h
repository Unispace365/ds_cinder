#pragma once
#ifndef EXXON_UI_VIDEO_INTERFACE_VIDEO_SCRUB_BAR
#define EXXON_UI_VIDEO_INTERFACE_VIDEO_SCRUB_BAR

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/video.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/button/image_button.h>

namespace ds {
class Globals;

/**
* \class exxon::VideoScrubBar
*		The seek bar controller / visualizer for a video
*/
class VideoScrubBar final : public ds::ui::Sprite {
public:

	VideoScrubBar(Globals& g, const float widdyToTheWidWid);

	void								linkVideo(ds::ui::Video* vid);

	virtual void						updateServer(const ds::UpdateParams& p);

	ds::ui::ImageButton*				mHome;
	ds::ui::ImageButton*				mPauseOn;
	ds::ui::ImageButton*				mPauseOff;
	ds::ui::ImageButton*				mVolumeOn;
	ds::ui::ImageButton*				mVolumeOff;
private:
	typedef ds::ui::Sprite				inherited;
	Globals&							mGlobals;


	ds::ui::Sprite*						mBacker;
	ds::ui::Sprite*						mProgress;
	ds::ui::Image*						mBall;
	ds::ui::Video*						mLinkedVideo;
	float								mCurrentProgress;

	bool								mIsPaused;
	double								mLastSeekTime;
	bool								mUpdatedPosition;
	ds::ui::TouchInfo::Phase			mTouchPhase;
	bool								mAdded;
	bool								mMoved;
	bool								mRemoved;


};

} // namespace exxon

#endif


