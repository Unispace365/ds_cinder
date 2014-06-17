#pragma once
#ifndef NA_APP_EVENTCLIENT_H_
#define NA_APP_EVENTCLIENT_H_

#include <functional>

namespace na {
class Event;
class Globals;

/**
 * \class na::EventClient
 * Utility for safely accessing the event mechanism.
 */
class EventClient {
  public:
    // This is being supplied a Globals, but it should be turned into a
    // notifier-owning object, so the EventClient can be moved into the framework.
    // Ideally, we'd like to avoid directly passing the actual notifier, so clients
    // won't do something stupid like try to use it without the EventClient, which
    // cleans up after itself in the destructor.
    EventClient(na::Globals&,
                // To be meaningful, clients should supply something
                // that handles notifications, or requests, or both.
                const std::function<void(const na::Event*)>& notifyListener,
                const std::function<void(na::Event&)>& requestListener = nullptr);
    ~EventClient();

    void            notify(const na::Event&);

  private:
    na::Globals&    mGlobals;
};

} // namespace na

#endif // NA_APP_EVENTCLIENT_H_
