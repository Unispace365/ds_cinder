#include <ds/app/event_notifier.h>

namespace ds {

/**
 * \class ds::EventNotifier
 */
EventNotifier::EventNotifier() {
}

EventNotifier::~EventNotifier() {
}

void EventNotifier::addListener(void *id, const std::function<void(const ds::Event*)>& fn) {
	mEventNotifier.addListener(id, fn);
}

void EventNotifier::addRequestListener(void *id, const std::function<void(ds::Event&)>& fn) {
	mEventNotifier.addRequestListener(id, fn);
}

void EventNotifier::removeListener(void *id) {
	mEventNotifier.removeListener(id);
}

void EventNotifier::removeRequestListener(void *id) {
	mEventNotifier.removeRequestListener(id);
}

void EventNotifier::notify(const ds::Event& e) {
	mEventNotifier.notify(&e);
}

void EventNotifier::request(ds::Event& e) {
	mEventNotifier.request(e);
}

void EventNotifier::setOnAddListenerFn(const std::function<ds::Event*(void)> &fn) {
	mEventNotifier.setOnAddListenerFn(fn);
}

} // namespace ds
