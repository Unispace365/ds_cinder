#include "stdafx.h"

#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ds/ui/interface_xml/interface_xml_importer.h"

namespace downstream {

StoryView::StoryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mTheVideo(nullptr)
{
	callAfterDelay([this] { restartVideo(); }, 2.0f);
}

void StoryView::restartVideo() {
	if(mTheVideo) {
		mTheVideo->release();
	}

	mTheVideo = nullptr;

	mTheVideo = new ds::ui::Video(mEngine);
	addChildPtr(mTheVideo);
	mTheVideo->loadVideo(ds::Environment::expand(mEngine.getAppSettings().getString("video:path")));
	
	mTheVideo->play();

	callAfterDelay([this] { restartVideo(); }, 1.0f);

}

} // namespace downstream

