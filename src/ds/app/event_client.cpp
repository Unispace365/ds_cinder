#include "event_client.h"
#include "event_client.h"
#include "event_client.h"
#include "stdafx.h"

#include <ds/app/event_client.h>
#include <ds/app/event_notifier.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {

/**
 * \class EventClient
 */
EventClient::EventClient(EventNotifier& n, const std::function<void(const ds::Event*)>& fn,
						 const std::function<void(ds::Event&)>& requestFn)
  : mNotifier(n) {
	if (fn) {
		n.mEventNotifier.addListener(this, [this, fn](const ds::Event* m) {
			if (this->mStopped) return;
			if (m) this->onAppEvent(*m);
			if (fn) fn(m);
		});
	} else {
		n.mEventNotifier.addListener(this, [this](const ds::Event* m) {
			if (this->mStopped) return;
			if (m) this->onAppEvent(*m);
		});
	}
	if (requestFn) n.mEventNotifier.addRequestListener(this, [this,requestFn](ds::Event& e){
			if (this->mStopped) return;
			requestFn(e);
	});
}

EventClient::EventClient(ds::ui::SpriteEngine& eng)
  : mNotifier(eng.getNotifier()) {
	mNotifier.mEventNotifier.addListener(this, [this](const ds::Event* m) {
		if (m) this->onAppEvent(*m);
	});
}

EventClient::EventClient(EventNotifier& notifier)
  : mNotifier(notifier) {
	mNotifier.mEventNotifier.addListener(this, [this](const ds::Event* m) {
		if (m) this->onAppEvent(*m);
	});
}



EventClient::~EventClient() {
	mNotifier.mEventNotifier.removeListener(this);
	mNotifier.mEventNotifier.removeRequestListener(this);
}

void EventClient::stop()
{
	mStopped = true;
}

void EventClient::start() {
	mStopped = false;
}

void EventClient::notify(const ds::Event& e) {
	if (mStopped) return;
	mNotifier.mEventNotifier.notify(&e);
}

void EventClient::notify(const std::string& eventName) {
	if (mStopped) return;
	mNotifier.notify(eventName);
}

void EventClient::request(ds::Event& e) {
	if (mStopped) return;
	mNotifier.mEventNotifier.request(e);
}

void EventClient::setNotifier(EventNotifier& notifier)
{
	mNotifier = notifier;
}

void EventClient::onAppEvent(const ds::Event& in_e) {
	if (mStopped) return;
	if (mEventCallbacks.empty()) return;

	auto callbackIt = mEventCallbacks.find(in_e.mWhat);
	if (callbackIt != end(mEventCallbacks)) {
		(callbackIt->second)(in_e);
	}
}

} // namespace ds
