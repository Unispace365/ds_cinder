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

  protected:
    friend class EventClient;

    ds2::Notifier<ds::Event>    mEventNotifier;
};

} // namespace ds

#endif // DS_APP_EVENTNOTIFIER_H
