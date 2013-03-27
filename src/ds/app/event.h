#pragma once
#ifndef DS_APP_EVENT_H_
#define DS_APP_EVENT_H_

#include <string>

namespace ds {

/**
 * \class ds::Event
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
 * \class ds::EventRegistry
 * Utility to make sure all event types are unique, and named.
 */
class EventRegistry {
  public:
    EventRegistry(const int what, const std::string& name);

    const int             mWhat;
};

} // namespace ds

#endif // DS_APP_EVENT_H_
