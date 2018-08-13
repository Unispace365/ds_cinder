#ifndef _RESOURCE_PARSER_APP_APPEVENTS_H_
#define _RESOURCE_PARSER_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace downstream {

class SomethingHappenedEvent : public ds::RegisteredEvent<SomethingHappenedEvent>{};

} // !namespace downstream

#endif // !_RESOURCE_PARSER_APP_APPEVENTS_H_
