#include "stdafx.h"

#include <memory>

#include "ds/app/event_notifier.h"

namespace ds {

/**
 * \class EventNotifier
 */
EventNotifier::EventNotifier() {
	mThreadId = std::this_thread::get_id();
}

void EventNotifier::addListener(void* id, const std::function<void(const Event*)>& fn) {
	mEventNotifier.addListener(id, fn);
}

void EventNotifier::addRequestListener(void* id, const std::function<void(Event&)>& fn) {
	mEventNotifier.addRequestListener(id, fn);
}

void EventNotifier::removeListener(void* id) {
	mEventNotifier.removeListener(id);
}

void EventNotifier::removeRequestListener(void* id) {
	mEventNotifier.removeRequestListener(id);
}

void EventNotifier::notify(const Event& e) {
	DS_LOG_VERBOSE(2, "EventNotifier::notify event " << e.getName());
	if (this->mThreadId != std::this_thread::get_id()) {
		DS_LOG_WARNING(
			"EventNotifier::notify called from a different thread than the one it was created on. This is not safe.");
	}
	mEventNotifier.notify(&e);
}


void EventNotifier::notify(const Event* e) {
	if (e) DS_LOG_VERBOSE(2, "EventNotifier::notify event " << e->getName());
	if (this->mThreadId != std::this_thread::get_id()) {
		DS_LOG_WARNING(
			"EventNotifier::notify called from a different thread than the one it was created on. This is not safe.");
	}
	mEventNotifier.notify(e);
}

void EventNotifier::notifyOnEngineThread(const std::shared_ptr<Event>& event) {
	// there is no std::dynamic_pointer_cast for unique_ptr in c++17; so we fake it.


	DS_LOG_VERBOSE(2, "EventNotifier::notifyOnEngineThread event " << event->getName());
	mEngine->timedCallback([this, event]() mutable { notify(event.get()); }, 0.1);
}


// void EventNotifier::notifyOnEngineThread(const ds::Event* e) {
//	DS_LOG_VERBOSE(2, "EventNotifier::notifyOnEngineThread event " << e->getName());
//	mEngine->timedCallback(
//		[this, event = e]() {
//			notify(event);
//			delete event;
//		},
//		0.1);
// }

void EventNotifier::notify(const std::string& eventName) {
	DS_LOG_VERBOSE(2, "EventNotifier::notify event " << eventName);
	if (this->mThreadId != std::this_thread::get_id()) {
		DS_LOG_WARNING(
			"EventNotifier::notify called from a different thread than the one it was created on. This is not safe.");
	}
	mEventNotifier.notify(event::Registry::get().getEventCreator(eventName)());
}

void EventNotifier::notifyOnEngineThread(const std::string& eventName) {
	DS_LOG_VERBOSE(2, "EventNotifier::notifyOnEngineThread event " << eventName);
	mEngine->timedCallback([this, eventAsString = eventName]() { notify(eventAsString); }, 0.1);
}

void EventNotifier::request(Event& event) {
	mEventNotifier.request(event);
}

void EventNotifier::setOnAddListenerFn(const std::function<Event*(void)>& fn) {
	mEventNotifier.setOnAddListenerFn(fn);
}

} // namespace ds
