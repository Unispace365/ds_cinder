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

	/*
	 * \fn as()
	 * \brief convenience to cast the Event to a derived type.
	 * \note you may add:
	 *    typename std::enable_if<std::is_base_of<ds::Event, T>::value>::type* = nullptr
	 * to the list of template parameters to make a compile-time check for
	 * Derived events but I found it unnecessary and makes the API dirty.
	 *
	 * \example local_event.as<MyDerivedEventType>()->mDreviedMember
	 */
	template<typename T>
	T* const				as();
	template<typename T>
	const T* const			as() const;

	int						mWhat;
};

// Template impl
template<typename T>
T* const Event::as()
{
	return dynamic_cast<T* const>(this);
}

template<typename T>
const T* const Event::as() const
{
	return dynamic_cast<const T* const>(this);
}
// End of Template impl


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
