#pragma once
#ifndef DS_APP_EVENTCLIENT_H
#define DS_APP_EVENTCLIENT_H

#include <functional>

namespace ds {
class Event;
class EventNotifier;

/**
 * \class ds::EventClient
 * Utility for safely accessing the event mechanism.
 */
class EventClient
{
  public:
    EventClient(EventNotifier&,
                  // To be meaningful, clients should supply something
                  // that handles notifications, or requests, or both.
                  const std::function<void(const ds::Event *)>& notifyListener,
                  const std::function<void(ds::Event &)>& requestListener = nullptr);
    ~EventClient();

    void            notify(const ds::Event&);

  private:
    EventNotifier&  mNotifier;
};

} // namespace ds

#endif // DS_APP_EVENTCLIENT_H
