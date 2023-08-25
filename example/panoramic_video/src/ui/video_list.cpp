#include "video_list.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/cfg/settings.h>
#include <ds/debug/logger.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include <ds/data/resource.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "ds/ui/interface_xml/interface_xml_importer.h"
#include "events/app_events.h"
#include "ui/video_list_item.h"

namespace panoramic {

VideoList::VideoList(Globals& g)
  : BasePanel(g.mEngine)
  , mGlobals(g)
  , mFileList(nullptr)
  , mScrollBar(nullptr)
  , mCloseButton(nullptr)
  , mEventClient(g.mEngine.getNotifier(), [this](const ds::Event* m) {
	  if (m) this->onAppEvent(*m);
  }) {
	const float itemSize = mEngine.getAppSettings().getFloat("info_list:item:height", 0, 100.0f);
	mFileList			 = new ds::ui::ScrollList(mEngine);

	mFileList->setItemTappedCallback([this](ds::ui::Sprite* bs, const ci::vec3& cent) {
		VideoListItem* rpi = dynamic_cast<VideoListItem*>(bs);
		if (rpi) {
			mEngine.getNotifier().notify(RequestPanoramicVideo(rpi->getInfo()));
		}
	});

	mFileList->setCreateItemCallback(
		[this, itemSize]() -> ds::ui::Sprite* { return new VideoListItem(mGlobals, mFileList->getWidth(), itemSize); });

	mFileList->setDataCallback([this](ds::ui::Sprite* bs, int dbId) {
		VideoListItem* rpi = dynamic_cast<VideoListItem*>(bs);
		if (rpi) {
			rpi->setInfo(mInfoMap[dbId]);
		}
	});

	mFileList->setAnimateOnCallback([this](ds::ui::Sprite* bs, const float delay) {
		VideoListItem* rpi = dynamic_cast<VideoListItem*>(bs);
		if (rpi) {
			rpi->animateOn(delay);
		}
	});

	mFileList->setStateChangeCallback([this](ds::ui::Sprite* bs, const bool highlighted) {
		VideoListItem* rpi = dynamic_cast<VideoListItem*>(bs);
		if (rpi) {
			rpi->setState(highlighted);
		}
	});
	mFileList->setLayoutParams(0.0f, 0.0f, itemSize + 2.0f);
	mFileList->setAnimateOnParams(0.0f, 0.025f);
	mFileList->getScrollArea()->setFadeColors(ci::ColorA(0.0f, 0.0f, 0.0f, 0.25f), ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));
	addChildPtr(mFileList);

	mScrollBar = new ds::ui::ScrollBar(mEngine, true);
	mScrollBar->linkScrollList(mFileList);
	addChildPtr(mScrollBar);

	const float padding = mEngine.getAppSettings().getFloat("titled_viewer:padding", 0, 20.0f);
	mCloseButton		= new ds::ui::ImageButton(mEngine, "%APP%/data/images/ui/close_button.png",
												  "%APP%/data/images/ui/close_button.png", padding * 2.0f);
	addChildPtr(mCloseButton);
	mCloseButton->getHighImage().setColor(ci::Color(0.5f, 0.5f, 0.5f));
	mCloseButton->setScale(0.5f);
	mCloseButton->setClickFn([this] { animateOff(); });

	ds::ui::Sprite* backy =
		new ds::ui::Sprite(mEngine, mCloseButton->getScaleWidth() * 2.0f, mCloseButton->getScaleHeight() * 2.0f);
	backy->setTransparent(false);
	backy->setColor(ci::Color::black());
	backy->setOpacity(0.5f);
	mCloseButton->addChildPtr(backy);
	backy->sendToBack();


	const float listPad		= mEngine.getAppSettings().getFloat("info_list:item:pad", 0, 10.0f);
	const float startWidth	= 600.0f;
	const float startHeight = mEngine.getAppSettings().getFloat("folder_browser:start_height", 0, 600.0f);
	mContentAspectRatio		= startWidth / startHeight;
	mLeftPad				= listPad;
	mRightPad				= listPad;
	mTopPad					= listPad;
	mBottomPad				= listPad;

	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth + mLeftPad + mRightPad, startHeight + mTopPad + mBottomPad);

	setColor(ci::Color(1.0f, 1.0f, 1.0f));
	setTransparent(false);

	setAnimateOnScript("grow; ease:outQuint");

	setData();
	setPosition(mEngine.getWorldWidth() / 2.0f - getWidth() / 2.0f,
				mEngine.getWorldHeight() / 2.0f - getHeight() / 2.0f);
}


void VideoList::onLayout() {
	if (mFileList) {
		mFileList->setSize(getWidth() - mLeftPad - mRightPad, getHeight() - mTopPad - mBottomPad);
		mFileList->setPosition(mLeftPad, mTopPad);

		mFileList->forEachLoadedSprite([this](ds::ui::Sprite* bs) {
			VideoListItem* rpi = dynamic_cast<VideoListItem*>(bs);
			if (rpi) {
				rpi->setSize(mFileList->getWidth(), rpi->getHeight());
				rpi->layout();
			}
		});

		if (mScrollBar) {
			mScrollBar->setSize(mScrollBar->getWidth(), mFileList->getHeight());
			mScrollBar->setPosition(-mScrollBar->getWidth(), mTopPad);
		}
	}

	if (mCloseButton) {
		mCloseButton->setPosition(getWidth(), getHeight() - mCloseButton->getScaleHeight());
	}
}

void VideoList::setData() {
	if (!mFileList) return;
	// clear out the ui until the new folder is loaded
	std::vector<int> productIds;
	mInfoMap.clear();
	int	 mediaId	 = 1;
	auto folderMedia = mGlobals.mAllData.mAllVideos;
	for (auto it = folderMedia.begin(); it < folderMedia.end(); ++it) {
		int thisId = mediaId++;
		productIds.push_back(thisId);
		mInfoMap[thisId] = (*it);
	}

	layout();
	mFileList->setContent(productIds);
}

void VideoList::onAppEvent(const ds::Event& in_e) {
	if (in_e.mWhat == RequestCloseAllEvent::WHAT() || in_e.mWhat == RequestPanoramicVideo::WHAT()) {
		animateOff();
	} else if (in_e.mWhat == RequestVideoList::WHAT()) {
		const RequestVideoList& e((const RequestVideoList&)in_e);
		tweenPosition(ci::vec3(e.mLocation.x - getWidth() / 2.0f, e.mLocation.y - getHeight() / 2.0f, 0.0f), 0.35f,
					  0.0f, ci::easeInOutQuint);
		animateOn();
	}
}

void VideoList::animateOn() {
	show();
	tweenOpacity(1.0f, 0.35f);
	//	tweenAnimateOn(true, 0.0f, 0.05f);
	tweenScale(ci::vec3(1.0f, 1.0f, 1.0f), 0.35f, 0.0f, ci::easeOutQuint);
}

void VideoList::animateOff() {
	tweenOpacity(0.0f, 0.35f);
	tweenScale(ci::vec3(), 0.35f, 0.0f, ci::easeInQuint, [this] { hide(); });
}


} // namespace panoramic
