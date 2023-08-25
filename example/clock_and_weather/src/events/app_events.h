#ifndef _CLOCK_AND_WEATHER_APP_APPEVENTS_H_
#define _CLOCK_AND_WEATHER_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace downstream {

class SomethingHappenedEvent : public ds::RegisteredEvent<SomethingHappenedEvent> {};

} // namespace downstream

#endif // !_CLOCK_AND_WEATHER_APP_APPEVENTS_H_
