#pragma once
#ifndef VIEWERS_MEDIA_MEDIA_INTERFACE_BUILDER
#define VIEWERS_MEDIA_MEDIA_INTERFACE_BUILDER

#include <string>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {
namespace ui {
class MediaInterface;

/**
*		A convenience for creating media interfaces for any player.
*		Handy if you're consuming any kind of player
*/
namespace MediaInterfaceBuilder {

/// Builds and links the appropriate kind of interface. 
/// mediaPlayer MUST be a PdfPlayer, VideoPlayer, or WebPlayer
MediaInterface*	buildMediaInterface(ds::ui::SpriteEngine& engine, ds::ui::Sprite* mediaPlayer, ds::ui::Sprite* parentSprite);

};

} // namespace ui
} // namespace ds

#endif
