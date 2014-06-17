#include "na/app/event_client.h"

#include "na/app/globals.h"

namespace na {

/**
 * \class na::EventClient
 */
EventClient::EventClient( na::Globals& g,
                          const std::function<void(const na::Event*)>& fn,
                          const std::function<void(na::Event&)>& requestFn)
  : mGlobals(g)
{
  if (fn) g.mEventNotifier.addListener(this, fn);
  if (requestFn) g.mEventNotifier.addRequestListener(this, requestFn);
}

EventClient::~EventClient()
{
  mGlobals.mEventNotifier.removeListener(this);
  mGlobals.mEventNotifier.removeRequestListener(this);
}

void EventClient::notify(const na::Event& e)
{
  DS_LOG_INFO_M("notify() event (" << e.getName() << ")", na::EVENT_LOG);
  mGlobals.mEventNotifier.notify(&e);
}

} // namespace na
