#ifndef _FULLSTARTER_APP_APPEVENTS_H_
#define _FULLSTARTER_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace fullstarter {

class SomethingHappenedEvent : public ds::RegisteredEvent<SomethingHappenedEvent>{};

} // !namespace fullstarter

#endif // !_FULLSTARTER_APP_APPEVENTS_H_