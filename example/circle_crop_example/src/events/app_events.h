#ifndef _CIRCLECROPEXAMPLE_APP_APPEVENTS_H_
#define _CIRCLECROPEXAMPLE_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace example {

class IdleStartedEvent : public ds::RegisteredEvent<IdleStartedEvent> {
  public:
	IdleStartedEvent(){};
};

class IdleEndedEvent : public ds::RegisteredEvent<IdleEndedEvent> {
  public:
	IdleEndedEvent(){};
};

} // namespace example

#endif // !_CIRCLECROPEXAMPLE_APP_APPEVENTS_H_