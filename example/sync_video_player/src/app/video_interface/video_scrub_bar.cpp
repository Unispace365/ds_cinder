#include "video_scrub_bar.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/text_defs.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include "app/globals.h"

namespace ds {

	VideoScrubBar::VideoScrubBar(Globals& g, const float widdyWamWamWozzle)
		: inherited(g.mEngine)
		, mGlobals(g)
		, mBacker(nullptr)
		, mProgress(nullptr)
		, mBall(nullptr)
		, mLinkedVideo(nullptr)
		, mHome(nullptr)
		, mPauseOn(nullptr)
		, mPauseOff(nullptr)
		, mVolumeOn(nullptr)
		, mVolumeOff(nullptr)
		, mCurrentProgress(0.0f)
		, mIsPaused(false)
		//, mUpdatedPosition(false)
		, mLastSeekTime(0.0)
		, mAdded(false)
		, mMoved(false)
		, mRemoved(false)
	{

		const float touchHeight = 60;
		const ci::Color progColor = ci::Color(1.0f, 0.0f, 0.0f);
			const float barHeight = 3;
			const float homeX = 150;
			const float pauseX = 80;
			const float volumeX = 0;
			 
		setSize(widdyWamWamWozzle, touchHeight);
		enable(true);
		enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti){
			//Performance is better if we pause the pipeline before scrubbing.  Will resume playing when done.
			if (!getParent() || !mLinkedVideo) return;
			mTouchPhase = ti.mPhase;
							//mUpdatedPosition = true;

			ci::vec3 loccy = globalToLocal(ti.mCurrentGlobalPoint);
			double newPercent = (double)(loccy.x / getWidth());
			if (newPercent < 0.0) newPercent = 0.0;
			if (newPercent > 1.0) newPercent = 1.0;
			if (ti.mPhase == ds::ui::TouchInfo::Added) {
				mAdded = true;
			}
			else if (ti.mPhase == ds::ui::TouchInfo::Moved) {
				mMoved = true;
			}
			else {
				mRemoved = true;
			}

			if (ti.mPhase == ds::ui::TouchInfo::Added || ti.mPhase == ds::ui::TouchInfo::Moved) {

				mLastSeekTime = newPercent;
			}

			//mUpdatedPosition = true;

		});
		mBacker = new ds::ui::Sprite(mEngine, widdyWamWamWozzle, barHeight);
		mBacker->setTransparent(false);
		mBacker->setColor(ci::Color::white());
		mBacker->enable(false);
		mBacker->setCenter(0.0f, 0.5f);
		mBacker->setPosition(0.0f, getHeight() / 2.0f);
		addChild(*mBacker);

		mProgress = new ds::ui::Sprite(mEngine, 0.0f, barHeight);
		mProgress->setTransparent(false);
		mProgress->setColor(progColor);
		mProgress->enable(false);
		mProgress->setCenter(0.0f, 0.5f);
		mProgress->setPosition(0.0f, getHeight() / 2.0f);
		addChild(*mProgress);


		mBall = new ds::ui::Image(mEngine, ds::Environment::expand("%APP%/data/images/ui/video_scrub_ball.png"), ds::ui::Image::IMG_CACHE_F | ds::ui::Image::IMG_PRELOAD_F);
		addChild(*mBall);
		mBall->setCenter(0.5f, 0.5f);
		mBall->setPosition(0.0f, getHeight() / 2.0f);
		mBall->setScale(0.5f, 0.5f);
		mBall->setColor(progColor);

		mHome = new ds::ui::ImageButton(mEngine, "%APP%/data/images/ui/video_scrbu_home.png", "%APP%/data/images/ui/video_scrbu_home.png", 20.0f);
		addChildPtr(mHome);
		mHome->setCenter(0.0f, 0.5f);
		mHome->setPosition(-homeX, getHeight() / 2.0f);

		mPauseOff = new ds::ui::ImageButton(mEngine, "%APP%/data/images/ui/video_scrbu_pause_off.png", "%APP%/data/images/ui/video_scrbu_pause_off.png", 20.0f);
		addChildPtr(mPauseOff);
		mPauseOff->setColor(progColor);
		mPauseOff->setCenter(0.0f, 0.5f);
		mPauseOff->setPosition(-pauseX, getHeight() / 2.0f);

		mPauseOn = new ds::ui::ImageButton(mEngine, "%APP%/data/images/ui/video_scrbu_pause_on.png", "%APP%/data/images/ui/video_scrbu_pause_on.png", 20.0f);
		addChildPtr(mPauseOn);
		mPauseOn->setColor(progColor);
		mPauseOn->hide();
		mPauseOn->setCenter(0.0f, 0.5f);
		mPauseOn->setPosition(-pauseX, getHeight() / 2.0f);

		mVolumeOn = new ds::ui::ImageButton(mEngine, "%APP%/data/images/ui/video_scrbu_volume_on.png", "%APP%/data/images/ui/video_scrbu_volume_on.png", 20.0f);
		addChildPtr(mVolumeOn);
		mVolumeOn->setCenter(0.0f, 0.5f);
		mVolumeOn->setPosition(widdyWamWamWozzle + volumeX, getHeight() / 2.0f);

		mVolumeOff = new ds::ui::ImageButton(mEngine, "%APP%/data/images/ui/video_scrbu_volume_off.png", "%APP%/data/images/ui/video_scrbu_volume_off.png", 20.0f);
		addChildPtr(mVolumeOff);
		mVolumeOff->hide();
		mVolumeOff->setCenter(0.0f, 0.5f);
		mVolumeOff->setPosition(mVolumeOn->getPosition().x, getHeight() / 2.0f);
}

void VideoScrubBar::linkVideo(ds::ui::Video* vid){
	mLinkedVideo = vid;
}

void VideoScrubBar::updateServer(const ds::UpdateParams& p){
	inherited::updateServer(p);
	if (!mLinkedVideo) return;

	if (mAdded || mMoved || mRemoved) {
		if (mAdded) {

			if (mLinkedVideo->getCurrentStatus() == ds::ui::GstVideo::Status::STATUS_PAUSED)
			{
				mIsPaused = true;
			}

			mLinkedVideo->pause();
			mAdded = false;
			mLinkedVideo->seekPosition(mLastSeekTime);

		}

		if ( mMoved){
			mLinkedVideo->seekPosition(mLastSeekTime);
			mMoved = false;
		}
		if (mRemoved){ //if  ds::ui::TouchInfo::Removed
			if (!mIsPaused){
				mLinkedVideo->play();
			}
			mIsPaused = false;
			mRemoved = false;
		}
	}
	
	if( mProgress && mBall){
		// update scrub bar
		float progress = (float)mLinkedVideo->getCurrentPosition();
		if(progress < 0.0f) progress = 0.0f;
		if(progress > 1.0f) progress = 1.0f;
		mProgress->setSize(progress * getWidth(), mProgress->getHeight());
		mBall->setPosition(progress * getWidth(), mBall->getPosition().y);
		mCurrentProgress = progress;
	}
}



} // namespace exxon


