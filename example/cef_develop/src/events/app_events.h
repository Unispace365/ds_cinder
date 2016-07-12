#ifndef _CEFDEVELOP_APP_APPEVENTS_H_
#define _CEFDEVELOP_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace cef {

class IdleStartedEvent : public ds::RegisteredEvent < IdleStartedEvent > {
public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent < IdleEndedEvent > {
public:
	IdleEndedEvent(){};

};

} // !namespace cef

#endif // !_CEFDEVELOP_APP_APPEVENTS_H_