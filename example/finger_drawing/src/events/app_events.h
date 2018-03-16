#ifndef _FINGER_DRAWING_APP_APPEVENTS_H_
#define _FINGER_DRAWING_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace example {

class StoryDataUpdatedEvent : public ds::RegisteredEvent<StoryDataUpdatedEvent>{
public:
	StoryDataUpdatedEvent(){};
}; 

class RequestAppExitEvent : public ds::RegisteredEvent<RequestAppExitEvent>{
public:
	RequestAppExitEvent(){};
};

} // !namespace example

#endif // !_FINGER_DRAWING_APP_APPEVENTS_H_