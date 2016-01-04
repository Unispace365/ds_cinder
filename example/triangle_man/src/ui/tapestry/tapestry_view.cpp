#include "tapestry_view.h"

#include <Poco/LocalDateTime.h>

#include <cinder/Rand.h>
#include <cinder/gl/gl.h>
#include <cinder/ImageIo.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace nwm {

TapestryView::SurfaceLoader::SurfaceLoader(){

}

void TapestryView::SurfaceLoader::run(){
	mImageSurface.reset();
	mImageSurface = ci::Surface8u(ci::loadImage(ds::Environment::expand(mImagePath)));
}


TapestryView::TapestryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mImage(nullptr)
	, mSurfaceQuery(g.mEngine, [this]{ return new SurfaceLoader(); })
{
	hide();
	setOpacity(0.0f);

	mImage = new ds::ui::Image(mEngine);
	addChildPtr(mImage);
	//mImage->hide();

	setImage("%APP%/data/images/ui/merica.png");
}

void TapestryView::onAppEvent(const ds::Event& in_e){
	
}

void TapestryView::setImage(const std::string& filename) {
	mSurfaceQuery.start([this, filename](SurfaceLoader& sl){ sl.mImagePath = filename; }, false);
	if(mImage){
		mImage->setImageFile(filename);
	}
}

void TapestryView::onSurfaceQuery(SurfaceLoader& l){
	mImageSurface = l.mImageSurface;

	animateOn();
}


void TapestryView::animateOn(){
	show();
	tweenOpacity(1.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f));
}

void TapestryView::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}

void TapestryView::updateServer(const ds::UpdateParams& p){
	ds::ui::Sprite::updateServer(p);

	// any changes for this frame happen here
}



} // namespace nwm
