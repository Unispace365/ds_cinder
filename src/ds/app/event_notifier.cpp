#include "stdafx.h"

#include <ds/app/event_notifier.h>


namespace ds {

/**
 * \class EventNotifier
 */
EventNotifier::EventNotifier() {
	mThreadId = std::this_thread::get_id();
}

EventNotifier::~EventNotifier() {}

void EventNotifier::addListener(void* id, const std::function<void(const ds::Event*)>& fn) {
	mEventNotifier.addListener(id, fn);
}

void EventNotifier::addRequestListener(void* id, const std::function<void(ds::Event&)>& fn) {
	mEventNotifier.addRequestListener(id, fn);
}

void EventNotifier::removeListener(void* id) {
	mEventNotifier.removeListener(id);
}

void EventNotifier::removeRequestListener(void* id) {
	mEventNotifier.removeRequestListener(id);
}

void EventNotifier::notify(const ds::Event& e) {
	DS_LOG_VERBOSE(2, "EventNotifier::notify event " << e.getName());
	if (this->mThreadId != std::this_thread::get_id()) {
		DS_LOG_WARNING(
			"EventNotifier::notify called from a different thread than the one it was created on. This is not safe.");
	}
	mEventNotifier.notify(&e);
}

void EventNotifier::notifyOnEngineThread(const ds::Event& e) {
	DS_LOG_VERBOSE(2, "EventNotifier::notifyOnEngineThread event " << e.getName());
	mEngine->timedCallback([this, event = e]() { notify(event); }, 0.1);
}

void EventNotifier::notify(const ds::Event* e) {
	if (e) DS_LOG_VERBOSE(2, "EventNotifier::notify event " << e->getName());
	if (this->mThreadId != std::this_thread::get_id()) {
		DS_LOG_WARNING(
			"EventNotifier::notify called from a different thread than the one it was created on. This is not safe.");
	}
	mEventNotifier.notify(e);
}

void EventNotifier::notifyOnEngineThread(const ds::Event* e) {
	DS_LOG_VERBOSE(2, "EventNotifier::notifyOnEngineThread event " << e->getName());
	mEngine->timedCallback([this, event = e]() { notify(event); }, 0.1);
}

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

void EventNotifier::request(ds::Event& e) {
	mEventNotifier.request(e);
}

void EventNotifier::setOnAddListenerFn(const std::function<ds::Event*(void)>& fn) {
	mEventNotifier.setOnAddListenerFn(fn);
}

} // namespace ds
