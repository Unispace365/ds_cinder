#include <ds/app/event_notifier.h>

namespace ds {

/**
 * \class ds::EventNotifier
 */
EventNotifier::EventNotifier()
{
}

EventNotifier::~EventNotifier()
{
}

void EventNotifier::addListener(void *id, const std::function<void(const ds::Event*)>& fn)
{
  mEventNotifier.addListener(id, fn);
}

void EventNotifier::notify(const ds::Event& e)
{
  mEventNotifier.notify(&e);
}

void EventNotifier::request(ds::Event& e)
{
  mEventNotifier.request(e);
}

} // namespace ds
