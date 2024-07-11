#include "stdafx.h"

#include "media_background.h"


#include <cinder/Rand.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/ui/media/media_player.h>
#include <ds/ui/media/player/video_player.h>
#include <ds/ui/sprite/video.h>

namespace waffles {

MediaBackground::MediaBackground(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/background/media_background.xml") {

	tweenAnimateOn(true, 0.0f, 0.025f);
}


} // namespace waffles
