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
// 	if(in_e.mWhat == IdleEndedEvent::WHAT()){
// 		const IdleEndedEvent& e((const IdleEndedEvent&)in_e);
// 		animateOn();
// 	} else if(in_e.mWhat == IdleStartedEvent::WHAT()){
// 		animateOff();
// 	}

	if(in_e.mWhat == RequestMediaOpenEvent::WHAT()){
		const RequestMediaOpenEvent& e((const RequestMediaOpenEvent&)in_e);
		addViewer(e.mMedia, e.mLocation, e.mStartWidth);
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



} // namespace mv
