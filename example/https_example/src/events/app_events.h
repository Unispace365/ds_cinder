#ifndef _HTTPS_EXAMPLE_APP_APPEVENTS_H_
#define _HTTPS_EXAMPLE_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace example {

class IdleStartedEvent : public ds::RegisteredEvent < IdleStartedEvent > {
public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent < IdleEndedEvent > {
public:
	IdleEndedEvent(){};

};

} // !namespace example

#endif // !_HTTPS_EXAMPLE_APP_APPEVENTS_H_