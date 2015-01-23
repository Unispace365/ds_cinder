#ifndef _FULLSTARTER_APP_APPEVENTS_H_
#define _FULLSTARTER_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace fullstarter {

class IdleStartedEvent : public ds::RegisteredEvent < IdleStartedEvent > {
public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent < IdleEndedEvent > {
public:
	IdleEndedEvent(){};

};

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_APPEVENTS_H_