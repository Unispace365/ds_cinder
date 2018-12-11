#pragma once
#ifndef _VIDEO_CONVERTER_APP_UI_PLAYBACK_VIEW_H_
#define _VIDEO_CONVERTER_APP_UI_PLAYBACK_VIEW_H_

#include <ds/ui/layout/smart_layout.h>

namespace downstream {
class ConversionItem;

/**
* \class downstream::PlaybackView
*			A view that shows a completed video
*/
class PlaybackView : public ds::ui::SmartLayout {
public:
	PlaybackView(ds::ui::SpriteEngine& eng);


};

} // namespace downstream

#endif

