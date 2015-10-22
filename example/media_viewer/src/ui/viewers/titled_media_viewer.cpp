#include "titled_media_viewer.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace mv {

TitledMediaViewer::TitledMediaViewer(Globals& g)
	: ds::ui::MediaViewer(g.mEngine, true)
	, mGlobals(g)

	, mBackground(nullptr)
	, mTitle(nullptr)
	, mBody(nullptr)
{
}

void TitledMediaViewer::setMedia(ds::model::MediaRef media) {
	loadMedia(media.getPrimaryResource());

	if(mTitle){
		mTitle->setText(media.getTitle());
	}

	if(mBody){
		mBody->setText(media.getBody());
	}

	onLayout();
}

void TitledMediaViewer::onLayout() {
	MediaViewer::onLayout();

	if(mBody && mBackground && mTitle){
		// do the stuff
	}
}


void TitledMediaViewer::animateOn(){
	show();
	tweenOpacity(1.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f));
}

void TitledMediaViewer::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}


} // namespace mv
