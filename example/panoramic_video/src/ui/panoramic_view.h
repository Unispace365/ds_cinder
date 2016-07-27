#pragma once
#ifndef DS_UI_VIEWERS_PANORAMIC_VIEW
#define DS_UI_VIEWERS_PANORAMIC_VIEW

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/panoramic_video.h>
#include <ds/app/event_client.h>

namespace panoramic {
class Globals;

class PanoramicView : public ds::ui::Sprite  {
public:
	PanoramicView(Globals& g);


protected:
	void								onAppEvent(const ds::Event&);
	void								clearVideo();
	void								startVideo(const ds::Resource& newVideo);

	Globals&							mGlobals;
	ds::ui::PanoramicVideo*				mPanoramicVideo;
	ds::EventClient						mEventClient;
};

} 

#endif
