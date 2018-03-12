#ifndef _FULLSTARTER_APP_APPEVENTS_H_
#define _FULLSTARTER_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace fullstarter {

class StoryDataUpdatedEvent : public ds::RegisteredEvent<StoryDataUpdatedEvent>{
public:
	StoryDataUpdatedEvent(){};
}; 

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_APPEVENTS_H_