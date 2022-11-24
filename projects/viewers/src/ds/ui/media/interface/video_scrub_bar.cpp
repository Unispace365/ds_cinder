#include "stdafx.h"

#include "video_scrub_bar.h"


#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include <ds/ui/media/player/youtube_player.h>
#include <ds/ui/sprite/pdf.h>
#include <ds/ui/sprite/video.h>

namespace ds { namespace ui {

	VideoScrubBar::VideoScrubBar(ds::ui::SpriteEngine& eng, const float heighty, const float buttHeight,
								 const ci::Color interfaceColor)
	  : ds::ui::Sprite(eng)
	  , mBacker(nullptr)
	  , mProgress(nullptr)
	  , mLinkedVideo(nullptr)
	  , mLinkedPdf(nullptr)
	  , mLinkedYouTube(nullptr) {

		// 	setTransparent(false);
		// 	setColor(ci::Color(0.0f, 0.5f, 0.0f));

		const float widdyWamWamWozzle = 400.0f;

		setSize(widdyWamWamWozzle, heighty);
		enable(true);
		enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti) {
			// seek to the relative position, or something
			if (!getParent()) return;

			ci::vec3 loccy		= globalToLocal(ti.mCurrentGlobalPoint);
			double	 newPercent = (double)(loccy.x / getWidth());
			if (newPercent < 0.0) newPercent = 0.0;
			if (newPercent > 1.0) newPercent = 1.0;
			if (mLinkedVideo) {
				mLinkedVideo->seekPosition(newPercent);
			}
			if (mLinkedPdf) {
				mLinkedPdf->setPageNum((int)roundf(newPercent * ((float)mLinkedPdf->getPageCount() + 1.0f)));
			}
			if (mLinkedYouTube) {
				mLinkedYouTube->seekPercent(newPercent);
			}
		});

		mBacker = new ds::ui::Sprite(mEngine, widdyWamWamWozzle, buttHeight / 2.0f);
		mBacker->setTransparent(false);
		mBacker->setColor(interfaceColor);
		mBacker->enable(false);
		mBacker->setCenter(0.0f, 0.5f);
		mBacker->setPosition(0.0f, getHeight() / 2.0f);
		mBacker->setOpacity(0.25f);
		addChild(*mBacker);

		mProgress = new ds::ui::Sprite(mEngine, 0.0f, buttHeight / 2.0f);
		mProgress->setTransparent(false);
		mProgress->setColor(interfaceColor);
		mProgress->enable(false);
		mProgress->setCenter(0.0f, 0.5f);
		mProgress->setPosition(0.0f, getHeight() / 2.0f);
		addChild(*mProgress);
	}

	void VideoScrubBar::linkVideo(ds::ui::GstVideo* vid) {
		mLinkedVideo = vid;
	}

	void VideoScrubBar::linkPdf(ds::ui::IPdf* linkedPdf) {
		mLinkedPdf = linkedPdf;
	}


	void VideoScrubBar::linkYouTube(ds::ui::YouTubeWeb* linkedYouTube) {
		mLinkedYouTube = linkedYouTube;
	}

	void VideoScrubBar::onUpdateServer(const ds::UpdateParams& p) {
		if (mLinkedVideo) {
			// update scrub bar
			setProgressPercent((float)mLinkedVideo->getCurrentPosition());


			if (mLinkedVideo->getIsStreaming()) {
				hide();
			} else {
				show();
			}
		}

		if (mLinkedPdf) {
			auto  curPage	  = (float)mLinkedPdf->getPageNum();
			auto  pageCount	  = (float)mLinkedPdf->getPageCount();
			float theProgress = 1.0f;
			if (pageCount > 1) theProgress = (curPage - 1) / (pageCount - 1);
			setProgressPercent(theProgress);
		}

		if (mLinkedYouTube) {
			setProgressPercent(mLinkedYouTube->getCurrentPercent());
		}
	}

	void VideoScrubBar::setProgressPercent(const float theProgress) {
		float progress = theProgress;
		if (progress < 0.0f) progress = 0.0f;
		if (progress > 1.0f) progress = 1.0f;
		if (mProgress) mProgress->setSize(progress * getWidth(), mProgress->getHeight());
		if (mNub) mNub->setPosition(progress * getWidth(), mNub->getPosition().y);
	}

	// don't change the size in layout
	void VideoScrubBar::layout() {
		if (mBacker) {
			mBacker->setSize(getWidth(), mBacker->getHeight());
		}
	}

	void VideoScrubBar::onSizeChanged() {
		layout();
	}

	ds::ui::Sprite* VideoScrubBar::getBacker() {
		return mBacker;
	}

	ds::ui::Sprite* VideoScrubBar::getProgress() {
		return mProgress;
	}

	void VideoScrubBar::addNub(ds::ui::Sprite* nub) {
		if (mNub) {
			auto n = mNub;
			n->release();
			mNub = nullptr;
		}
		mNub = nub;
		addChildPtr(mNub);
	}
}} // namespace ds::ui
