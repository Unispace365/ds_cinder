#ifndef _PARTICLE_EFFECTS_APP_APPEVENTS_H_
#define _PARTICLE_EFFECTS_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace example {

class SomethingHappenedEvent : public ds::RegisteredEvent<SomethingHappenedEvent>{};

struct ImageSwapEvent : public ds::RegisteredEvent<ImageSwapEvent> {
	ImageSwapEvent(std::string& path) : mPath(path) {}
	std::string mPath;
};

} // !namespace example

#endif // !_PARTICLE_EFFECTS_APP_APPEVENTS_H_
