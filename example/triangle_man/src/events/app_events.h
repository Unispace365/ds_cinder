#ifndef _TRIANGLE_MAN_APP_APPEVENTS_H_
#define _TRIANGLE_MAN_APP_APPEVENTS_H_

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

} // !namespace nwm

#endif // !_TRIANGLE_MAN_APP_APPEVENTS_H_

