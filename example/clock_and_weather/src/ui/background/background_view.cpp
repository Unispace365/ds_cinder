#include "stdafx.h"

#include "background_view.h"

#include <poco/File.h>

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/media/media_player.h>

#include "events/app_events.h"

namespace {
struct Init {
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			e.registerSpriteImporter("background_view", [](ds::ui::SpriteEngine& enginey) -> ds::ui::Sprite* {
				return new downstream::BackgroundView(enginey);
			});		
		});
	}
};

Init INIT;
}  // namespace


namespace downstream {

BackgroundView::BackgroundView(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "background_view.xml")
	, mCurBackground(nullptr)
{
	auto pathy = ds::Environment::expand("%APP%/data/images/backgrounds/");
	Poco::File theDir = Poco::File(pathy);
	theDir.list(mBackgrounds);

	std::random_shuffle(mBackgrounds.begin(), mBackgrounds.end());

	cycleBackground();
}


void BackgroundView::cycleBackground() {
	if(mBackgrounds.empty()) return;

	if(mCurBackground) {
		auto cb = mCurBackground;
		mCurBackground->tweenAnimateOff(true, 0.0f, 0.05f, [cb] { cb->release(); });
		mCurBackground = nullptr;
	}

	mCurBackgroundIndex++;
	if(mCurBackgroundIndex > mBackgrounds.size() - 1) {
		std::random_shuffle(mBackgrounds.begin(), mBackgrounds.end());
		mCurBackgroundIndex = 0;
	}

	mCurBackground = new ds::ui::SmartLayout(mEngine, "background_item.xml");
	addChildPtr(mCurBackground);
	auto mp = mCurBackground->getSprite<ds::ui::MediaPlayer>("the_media");
	if(mp) {
		auto pathy = ds::Environment::expand("%APP%/data/images/backgrounds/");
		mp->loadMedia(pathy + mBackgrounds[mCurBackgroundIndex]);
		runLayout();
	}

	mCurBackground->tweenAnimateOn(true, 0.0f, 0.05f);

	callAfterDelay([this] { cycleBackground(); }, mEngine.getAppSettings().getDouble("background:cylce_time", 0, 10.0));
}

} // namespace downstream

