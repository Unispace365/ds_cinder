#pragma once
#ifndef DS_APP_EVENT_H_
#define DS_APP_EVENT_H_

#include "event_registry.h"

namespace ds {

/**
 * \class ds::Event
 * \brief Abstract message class. Ignore this, and derive from RegisteredEvent.
 *
 * EXAMPLE DECLARATION of an event class:

	class ChangeEvent : public RegisteredEvent<ChangeEvent> {
	public:
		ChangeEvent() { }
	};
 */
class Event {
public:
	Event();
	Event(const int what);
	virtual ~Event();

	const std::string&		getName() const;

	int						mWhat;
};

/**
* \class ds::event::RegisteredEvent
* \brief All events should derive from this class, to be properly
* registered with the system.
*/
template<class Derived>
class RegisteredEvent : public Event {
public:
	// Unique identifier for this message
	static int						WHAT() { return sENTRY.getWhat(); }
	// Unique channel name for this message
	static const std::string&		CHANNEL() { return sENTRY.getChannel(); }

protected:
	RegisteredEvent()				: Event(sENTRY.getWhat()) { }

private:
	static event::Registry::Entry	sENTRY;
};

template<class Derived>
event::Registry::Entry RegisteredEvent<Derived>::sENTRY(typeid(Derived).name());

} // namespace ds

#endif
