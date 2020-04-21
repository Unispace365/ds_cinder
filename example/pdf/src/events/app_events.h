#ifndef _PDF_EXAMPLE_APP_APPEVENTS_H_
#define _PDF_EXAMPLE_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace downstream {

class SomethingHappenedEvent : public ds::RegisteredEvent<SomethingHappenedEvent>{};

} // !namespace downstream

#endif // !_PDF_EXAMPLE_APP_APPEVENTS_H_
