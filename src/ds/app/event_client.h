#pragma once
#ifndef DS_APP_EVENTCLIENT_H
#define DS_APP_EVENTCLIENT_H

#include <functional>
#include <initializer_list>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ds {
class Event;
class EventNotifier;
} // namespace ds

namespace ds::ui {
class SpriteEngine;
} // namespace ds::ui

namespace ds {

/**
 * \class EventClient
 * Utility for safely accessing the event mechanism.
 */
class EventClient {
  public:
	/// Uses sprite engine's default event notifier. Use the listenToEvents() callback with this constructor
	EventClient();

	EventClient(const ui::SpriteEngine&);

	/// Uses sprite engine's default event notifier. Use the listenToEvents() callback with this constructor
	EventClient(EventNotifier&);


	/// Supply your own event notifier and listeners
	/// To be meaningful, clients should supply something
	/// that handles notifications, or requests, or both.
	EventClient(EventNotifier& n, const std::function<void(const Event*)>& fn,
				const std::function<void(Event&)>& requestFn = nullptr);
	~EventClient();

	EventClient(const EventClient&)			   = delete;
	EventClient(EventClient&&)				   = delete;
	EventClient& operator=(const EventClient&) = delete;
	EventClient& operator=(EventClient&&)	   = delete;

	void stop();
	void start();

	void notify(const Event&) const;
	void notify(const std::string& eventName) const;
	void request(Event&) const;


	/// Calls the lambda callback for the event type from Template, casting event automatically
	/// This is an alternative to supplying a listener callback in the constructor for all events
	template <class EVENT>
	void listenToEvents(std::function<void(const EVENT&)> callback) {
		static_assert(std::is_base_of<Event, EVENT>::value, "EVENT not derived from ds::Event");
		const auto		 type = EVENT::WHAT();
		std::unique_lock lock(mEventsMtx);
		mEventCallbacks[type] = [callback](const Event& e) {
			callback(static_cast<const EVENT&>(e));
		};
	}
	/// Disables / removes callback (if it exists) for the event from the template
	/// This doesn't affect the callback supplied in the constructor
	template <class EVENT>
	void stopListeningToEvents() {
		static_assert(std::is_base_of<Event, EVENT>::value, "EVENT not derived from ds::Event");
		auto			 type = EVENT::WHAT();
		std::unique_lock lock(mEventsMtx);
		auto			 findy = mEventCallbacks.find(type);
		if (findy != end(mEventCallbacks)) {
			mEventCallbacks.erase(findy);
		}
	}

	// change the notifier for this client. allows the client to listen to channels
	void setNotifier(EventNotifier& notifier);

	// static convenience functions for listening and removing to events from multiple clients
	template <class EVENT>
	static void clientsListenToEvents(std::function<void(const EVENT&)>	  callback,
									  std::initializer_list<EventClient*> list);

	template <class EVENT>
	static void clientsStopListeningToEvents(std::initializer_list<EventClient*> list);

  private:
	bool		   mStopped	 = false;
	EventNotifier* mNotifier = nullptr;

	using EventCallback = std::function<void(const Event&)>;
	using EventMap		= std::unordered_map<size_t, EventCallback>;

	void	   onAppEvent(const Event&);
	EventMap   mEventCallbacks;
	std::mutex mEventsMtx;
};


// static convenience functions for listening and removing to events from multiple clients

template <class EVENT>
inline void EventClient::clientsListenToEvents(std::function<void(const EVENT&)>   callback,
											   std::initializer_list<EventClient*> list) {
	static_assert(std::is_base_of<Event, EVENT>::value, "EVENT not derived from ds::Event");
	std::vector<EventNotifier*> notifiers;
	for (auto client : list) {

		if (client) {
			if (std::find(notifiers.begin(), notifiers.end(), client->mNotifier) == notifiers.end()) {

				notifiers.push_back(client->mNotifier);
				client->listenToEvents<EVENT>(callback);
			}
		}
	}
}

template <class EVENT>
inline void EventClient::clientsStopListeningToEvents(std::initializer_list<EventClient*> list) {
	static_assert(std::is_base_of<Event, EVENT>::value, "EVENT not derived from ds::Event");
	for (auto client : list) {
		if (client) {
			client->stopListeningToEvents<EVENT>();
		}
	}
}

} // namespace ds

#endif // DS_APP_EVENTCLIENT_H
