#include "viewer_controller.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ui/viewers/titled_media_viewer.h"

namespace mv {

ViewerController::ViewerController(Globals& g)
	: inherited(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
{
}

void ViewerController::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == RequestMediaOpenEvent::WHAT()){
		const RequestMediaOpenEvent& e((const RequestMediaOpenEvent&)in_e);
		addViewer(e.mMedia, e.mLocation, e.mStartWidth);
	} else if(in_e.mWhat == RequestCloseAllEvent::WHAT()){
		const float deltaAnim = 0.05f;
		float delayey = 0.0f;
		for(auto it = mViewers.begin(); it < mViewers.end(); ++it){
			animateViewerOff((*it), delayey);
			delayey += deltaAnim;
		}
	} else if(in_e.mWhat == RequestLayoutEvent::WHAT()){
		layoutViewers();
	}
}

void ViewerController::updateServer(const ds::UpdateParams& p){
	inherited::updateServer(p);

	// any changes for this frame happen here
}

void ViewerController::addViewer(ds::model::MediaRef newMedia, const ci::Vec3f location, const float startWidth) {
	TitledMediaViewer* tmv = new TitledMediaViewer(mGlobals);
	addChildPtr(tmv);
	mViewers.push_back(tmv);
	tmv->setMedia(newMedia);
	tmv->setPosition(location);
	tmv->setViewerWidth(startWidth);
	tmv->animateOn();
}

void ViewerController::animateViewerOff(TitledMediaViewer* viewer, const float delayey) {
	if(!viewer) return;

	viewer->tweenStarted();
	viewer->tweenPosition(ci::Vec3f(viewer->getPosition().x + viewer->getWidth()/4.0f, viewer->getPosition().y + 100.0f + viewer->getHeight()/4.0f, viewer->getPosition().z), mGlobals.getAnimDur(), delayey, ci::EaseInQuad());
	viewer->tweenOpacity(0.0f, mGlobals.getAnimDur(), delayey, ci::EaseInQuad());
	viewer->tweenScale(viewer->getScale() / 2.0f, mGlobals.getAnimDur(), delayey, ci::EaseInQuad(), [this, viewer](){
		removeViewer(viewer);
	});
}

void ViewerController::removeViewer(TitledMediaViewer* viewer) {
	for(auto it = mViewers.begin(); it < mViewers.end(); ++it){
		if((*it) == viewer){
			mViewers.erase(it);
			break;
		}
	}

	if(viewer){
		viewer->exit();
		viewer->release();
	}
}

void ViewerController::layoutViewers() {

}



} // namespace mv
