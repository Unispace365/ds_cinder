#include "stdafx.h"

#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace downstream {

StoryView::StoryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mThePdf(nullptr)
{
	callAfterDelay([this] { restartSprite(); }, 2.0f);
}



void StoryView::restartSprite() {

	/*
	ci::gl::Texture::Format formatty;
	formatty.setInternalFormat(GL_RGB);
	formatty.setMinFilter(GL_LINEAR);
	formatty.setMagFilter(GL_LINEAR);
	mTexture = ci::gl::Texture::create(1920, 1080, formatty);
	callAfterDelay([this] { restartSprite(); }, 0.01);

	return;
	*/

	if(mThePdf) {
		auto tt = mThePdf;
		mThePdf->callAfterDelay([this, tt] { tt->release(); }, 0.1f);
	}

	mThePdf = nullptr;

	mThePdf = new ds::ui::Pdf(mEngine);
	addChildPtr(mThePdf);
	mThePdf->setResourceFilename(ds::Environment::expand(mGlobals.getAppSettings().getString("pdf:path")));

//	callAfterDelay([this] { mEngine.getNotifier().notify(RequestAppExitEvent()); }, 5.5);
	callAfterDelay([this] { restartSprite(); }, 0.5);

}
} // namespace downstream

