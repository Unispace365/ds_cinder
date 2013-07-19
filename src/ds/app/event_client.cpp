#include <ds/app/event_client.h>
#include <ds/app/event_notifier.h>

namespace ds {

/**
 * \class ds::EventClient
 */
EventClient::EventClient( EventNotifier& n,
                          const std::function<void(const ds::Event *)>& fn,
                          const std::function<void(ds::Event &)>& requestFn)
		: mNotifier(n) {
	if (fn) n.mEventNotifier.addListener(this, fn);
	if (requestFn) n.mEventNotifier.addRequestListener(this, requestFn);
}

EventClient::~EventClient() {
	mNotifier.mEventNotifier.removeListener(this);
	mNotifier.mEventNotifier.removeRequestListener(this);
}

void EventClient::notify(const ds::Event& e) {
	mNotifier.mEventNotifier.notify(&e);
}

void EventClient::request(ds::Event& e) {
	mNotifier.mEventNotifier.request(e);
}

} // namespace ds
