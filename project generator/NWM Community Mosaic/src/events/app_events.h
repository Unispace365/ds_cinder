#ifndef _NWM COMMUNITY MOSAIC_APP_APPEVENTS_H_
#define _NWM COMMUNITY MOSAIC_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace nwm {

class IdleStartedEvent : public ds::RegisteredEvent < IdleStartedEvent > {
public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent < IdleEndedEvent > {
public:
	IdleEndedEvent(){};

};

class StoryDataUpdatedEvent : public ds::RegisteredEvent<StoryDataUpdatedEvent>{
public:
	StoryDataUpdatedEvent(){};
}; 

class RequestAppExitEvent : public ds::RegisteredEvent<RequestAppExitEvent>{
public:
	RequestAppExitEvent(){};
};

} // !namespace nwm

#endif // !_NWM COMMUNITY MOSAIC_APP_APPEVENTS_H_