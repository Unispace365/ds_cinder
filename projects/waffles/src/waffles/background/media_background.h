#pragma once

#include <ds/ui/layout/smart_layout.h>


namespace waffles {

/**
 * \class waffles::MediaBackground
 *			Background that's a media item
 */
class MediaBackground : public ds::ui::SmartLayout {
  public:
	MediaBackground(ds::ui::SpriteEngine& g);
};

} // namespace waffles
