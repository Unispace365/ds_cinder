#include "stdafx.h"

#include "playback_view.h"

#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>

#include <ds/ui/button/sprite_button.h>
#include <ds/ui/media/media_player.h>

#include "events/app_events.h"



namespace downstream {

PlaybackView::PlaybackView(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "playback_view.xml")
{
	listenToEvents<PlayVideoRequest>([this](auto& e) {
		auto theMp = getSprite<ds::ui::MediaPlayer>("the_player");
		if(theMp) {
			theMp->loadMedia(e.mPath);
			runLayout();
			show();
		}

	});

	auto close_button = getSprite<ds::ui::SpriteButton>("close_button.the_button");
	if(close_button) {
		close_button->setClickFn([this] {

			auto theMp = getSprite<ds::ui::MediaPlayer>("the_player");
			if(theMp) {
				theMp->stopContent();
			}

			hide();
		});
	}

	//hide();
}


} // namespace downstream

