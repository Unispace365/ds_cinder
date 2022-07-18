#pragma once
#ifndef DS_CONTENT_CONTENT_EVENTS
#define DS_CONTENT_CONTENT_EVENTS

#include <ds/app/event.h>

namespace ds {

/// ContentQuery has completed and there is new content available
class ContentUpdatedEvent : public ds::RegisteredEvent<ContentUpdatedEvent> {};

/// A request to re-query content (all queries are asynchronous)
class RequestContentQueryEvent : public ds::RegisteredEvent<RequestContentQueryEvent> {};

/// A message was received from dsnode on localhost port 7777
class DsNodeMessageReceivedEvent : public ds::RegisteredEvent<DsNodeMessageReceivedEvent> {};

struct CmsDataLoadCompleteEvent : public ds::RegisteredEvent<CmsDataLoadCompleteEvent> {
};

struct ScheduleUpdatedEvent : public ds::RegisteredEvent<ScheduleUpdatedEvent> {};

} // !namespace ds

#endif 
