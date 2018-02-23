#ifndef _GLITCH_SHADERS_APP_APPEVENTS_H_
#define _GLITCH_SHADERS_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace downstream {

class StoryDataUpdatedEvent : public ds::RegisteredEvent<StoryDataUpdatedEvent>{
public:
	StoryDataUpdatedEvent(){};
}; 

class RequestAppExitEvent : public ds::RegisteredEvent<RequestAppExitEvent>{
public:
	RequestAppExitEvent(){};
};

} // !namespace downstream

#endif // !_GLITCH_SHADERS_APP_APPEVENTS_H_
