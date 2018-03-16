#ifndef _GETTING_STARTED_APP_APPEVENTS_H_
#define _GETTING_STARTED_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace downstream {

class SlideSetRequest : public ds::RegisteredEvent<SlideSetRequest> {};
class SlideBackRequest : public ds::RegisteredEvent<SlideBackRequest> {};
class SlideForwardRequest : public ds::RegisteredEvent<SlideForwardRequest> {};

} // !namespace downstream

#endif // !_GETTING_STARTED_APP_APPEVENTS_H_
