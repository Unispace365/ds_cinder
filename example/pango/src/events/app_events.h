#ifndef _PANGO_APP_APPEVENTS_H_
#define _PANGO_APP_APPEVENTS_H_

#include <ds/app/event.h>

namespace pango {


class StoryDataUpdatedEvent : public ds::RegisteredEvent<StoryDataUpdatedEvent> {
  public:
	StoryDataUpdatedEvent(){};
};

} // namespace pango

#endif // !_PANGO_APP_APPEVENTS_H_
