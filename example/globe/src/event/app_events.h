#ifndef _GLOBE_EXAMPLE_APP_APPEVENTS_H_
#define _GLOBE_EXAMPLE_APP_APPEVENTS_H_

#include "app/globals.h"
#include <ds/app/event.h>

namespace globe_example {

class GlobeFilterChange : public ds::RegisteredEvent<GlobeFilterChange> {
  public:
	GlobeFilterChange(int id)
	  : mFilterId(id){};
	const int mFilterId;
};


class GlobeFocusEvent : public ds::RegisteredEvent<GlobeFocusEvent> {
  public:
	GlobeFocusEvent(float lat, float longitude)
	  : mLatitude(lat)
	  , mLongitude(longitude){};
	const float mLatitude;
	const float mLongitude;
};


} // namespace globe_example

#endif // !_REDHATRECEPTIONTABLEAPP_APP_APPEVENTS_H_