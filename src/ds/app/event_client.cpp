#include "stdafx.h"

#include "ds/app/event_client.h"
#include "ds/app/event_notifier.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {

/**
 * \class EventClient
 */

EventClient::EventClient() {
	mNotifier = nullptr;
}

EventClient::EventClient(EventNotifier& n, const std::function<void(const Event*)>& fn,
						 const std::function<void(Event&)>& requestFn)
  : mNotifier(&n) {
	if (fn) {
		n.mEventNotifier.addListener(this, [this, fn](const Event* m) {
			if (this->mStopped) return;
			if (m) this->onAppEvent(*m);
			if (fn) fn(m);
		});
	} else {
		n.mEventNotifier.addListener(this, [this](const Event* m) {
			if (this->mStopped) return;
			if (m) this->onAppEvent(*m);
		});
	}
	if (requestFn)
		n.mEventNotifier.addRequestListener(this, [this, requestFn](Event& e) {
			if (this->mStopped) return;
			requestFn(e);
		});
}

EventClient::EventClient(const ui::SpriteEngine& eng)
  : mNotifier(&eng.getNotifier()) {
	mNotifier->mEventNotifier.addListener(this, [this](const Event* m) {
		if (m) this->onAppEvent(*m);
	});
}

EventClient::EventClient(EventNotifier& notifier)
  : mNotifier(&notifier) {
	mNotifier->mEventNotifier.addListener(this, [this](const Event* m) {
		if (m) {
			this->onAppEvent(*m);
		}
	});
}


EventClient::~EventClient() {
	if (mNotifier) {
		mNotifier->mEventNotifier.removeListener(this);
		mNotifier->mEventNotifier.removeRequestListener(this);
	}
}

void EventClient::stop() {
	mStopped = true;
}

void EventClient::start() {
	mStopped = false;
}

void EventClient::notify(const Event& e) const {
	if (mStopped) return;
	if (!mNotifier) return;
	mNotifier->mEventNotifier.notify(&e);
}

void EventClient::notify(const std::string& eventName) const {
	if (mStopped) return;
	if (!mNotifier) return;
	mNotifier->notify(eventName);
}

void EventClient::request(Event& e) const {
	if (mStopped) return;
	if (!mNotifier) return;
	mNotifier->mEventNotifier.request(e);
}

void EventClient::setNotifier(EventNotifier& notifier) {
	if (mNotifier == &notifier) {
		return;
	}
	if (mNotifier) {
		mNotifier->mEventNotifier.removeListener(this);
		mNotifier->mEventNotifier.removeRequestListener(this);
	}
	mNotifier = &notifier;
	if (mNotifier) {
		mNotifier->mEventNotifier.addListener(this, [this](const Event* m) {
			if (m) this->onAppEvent(*m);
		});
	}
}

void EventClient::onAppEvent(const Event& in_e) {
	if (mStopped) return;
	if (mEventCallbacks.empty()) return;
	std::unordered_map<size_t, EventCallback> cbs;
	{
		std::unique_lock lock(mEventsMtx);
		cbs = mEventCallbacks;
	}
	auto callbackIt = cbs.find(in_e.mWhat);
	if (callbackIt != end(cbs)) {
		(callbackIt->second)(in_e);
	}
}

} // namespace ds
