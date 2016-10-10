#pragma once
#ifndef DS_APP_EVENT_H_
#define DS_APP_EVENT_H_

#include "event_registry.h"
#include <cinder/Vector.h>

namespace ds {
namespace ui {
class Sprite;
}

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

	/** An event-specific parameter, for client usage. Generally global space. May be empty	*/
	ci::vec3				mEventOrigin;
	/** An event-specific parameter, for client usage. Defaults to nullptr	*/
	ds::ui::Sprite*			mSpriteOriginator;
	/** An event-specific ID. Could be used to lookup info from a db, etc. Default=0*/
	int						mUserId;
	/** An event-specific size. For instance if you want launch a panel at a certain width */
	ci::vec3				mUserSize;
	/** An event-specific string. Defaults to empty */
	std::string				mUserStringData;
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


// Only works in Visual Studio. GCC will likely mangle type names differently, sorry.
static const std::string demangleTypeName(const std::string& n){
	return n.substr(n.find_last_of(':') + 1);
}

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

	static const std::string		NAME() { return demangleTypeName(typeid(Derived).name()); }

protected:
	RegisteredEvent() : Event(sENTRY.getWhat()) {}

private:
	static event::Registry::Entry	sENTRY;
};

template<class Derived>
event::Registry::Entry RegisteredEvent<Derived>::sENTRY(NAME());

} // namespace ds

#endif
