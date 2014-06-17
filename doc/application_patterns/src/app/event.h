#pragma once
#ifndef NA_APP_EVENT_H_
#define NA_APP_EVENT_H_

#include <string>

namespace na {

/**
 * \class na::Event
 */
class Event {
  public:
    Event();
    Event(const int what);
    virtual ~Event();

    const std::string&          getName() const;

    int                         mWhat;
};

/**
 * \class na::EventRegistry
 * Utility to make sure all event types are unique, and named.
 */
class EventRegistry {
  public:
    EventRegistry(const int what, const std::string& name);

    const int             mWhat;
};

} // namespace na

#endif // NA_APP_EVENT_H_
