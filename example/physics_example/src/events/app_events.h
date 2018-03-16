#ifndef _PHYSICS_EXAMPLE_APP_APPEVENTS_H_
#define _PHYSICS_EXAMPLE_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace physics {

class StoryDataUpdatedEvent : public ds::RegisteredEvent<StoryDataUpdatedEvent>{
public:
	StoryDataUpdatedEvent(){};
}; 


} // !namespace physics

#endif // !_PHYSICS_EXAMPLE_APP_APPEVENTS_H_
