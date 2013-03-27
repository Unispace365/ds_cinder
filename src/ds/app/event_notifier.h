#pragma once
#ifndef DS_APP_EVENTNOTIFIER_H
#define DS_APP_EVENTNOTIFIER_H

#include <ds/app/event.h>
#include <ds/util/notifier_2.h>

namespace ds {

/**
 * \class ds::EventNotifier
 * Holder for an event notifier.
 */
class EventNotifier
{
  public:
    EventNotifier();
    virtual ~EventNotifier();

    void                        addListener(void *id, const std::function<void(const ds::Event*)>&);

    // Send an event to the system, for clients that don't need
    // an EventClient (i.e. don't need to receive events)
    void                        notify(const ds::Event&);
    // Request information from the system.
    void                        request(ds::Event&);

  protected:
    friend class EventClient;

    ds2::Notifier<ds::Event>    mEventNotifier;
};

} // namespace ds

#endif // DS_APP_EVENTNOTIFIER_H
