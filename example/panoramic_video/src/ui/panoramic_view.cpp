#include "panoramic_view.h"

#include <ds/app/event_notifier.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/ui/scroll/scroll_area.h>

#include <ds/data/resource.h>

#include "app/globals.h"
#include "app/app_defs.h"
#include "events/app_events.h"

namespace panoramic {

PanoramicView::PanoramicView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mPanoramicVideo(nullptr)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
{
	
}

void PanoramicView::clearVideo(){
	if(mPanoramicVideo){
		auto pv = mPanoramicVideo;
		mPanoramicVideo->tweenOpacity(0.0f, 0.5f, 0.0f, ci::easeNone, [this, pv]{ pv->release(); });
		mPanoramicVideo = nullptr;
	}
}

void PanoramicView::startVideo(const ds::Resource& newVideo){
	clearVideo();
	if(!mPanoramicVideo){
		mPanoramicVideo = new ds::ui::PanoramicVideo(mEngine);
		addChildPtr(mPanoramicVideo);
	}

	mPanoramicVideo->setSize(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	mPanoramicVideo->setFOV(mGlobals.getSettingsLayout().getFloat("panoramic_video:fov", 0, 60.0f));
	mPanoramicVideo->loadVideo(newVideo.getAbsoluteFilePath());
	mPanoramicVideo->setOpacity(0.0f);
	mPanoramicVideo->tweenOpacity(1.0f, 1.0f, 1.0f);
}

void PanoramicView::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == RequestCloseAllEvent::WHAT()){
		clearVideo();
	} else if(in_e.mWhat == RequestPanoramicVideo::WHAT()){
		const RequestPanoramicVideo& e((const RequestPanoramicVideo&)in_e);
		startVideo(e.mResource);
	} else if(in_e.mWhat == RequestVideoList::WHAT()){
		// anything?
	}
}


} // namespace mv


