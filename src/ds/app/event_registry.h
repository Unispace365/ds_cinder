#pragma once
#ifndef DS_APP_EVENTREGISTRY_H_
#define DS_APP_EVENTREGISTRY_H_

#include <map>
#include <string>
#include <functional>


namespace ds {

class Event;

namespace event {

/**
 * \class ds::event::Registry
 * \brief Store all registered message types. This is an internal-only
 * object; entries are made automatically by constructing new
 * RegisteredEvent subclasses.
 */
class Registry {
public:
	static Registry&			get();
	Registry();

	int							add(const std::string&);

	// Print out all registered messages
	void						report();

	const std::string&			getName(const int what);

	void						addEventCreator(const std::string& eventName, std::function<ds::Event*()> creator);
	std::function<ds::Event*()>	getEventCreator(const std::string& eventName);

public:
	class Entry {
	public:
		Entry(const std::string &name);

		int						getWhat() const { return mWhat; }
		const std::string&		getChannel() const { return mChannel; }
		const std::string&		getName() const { return mName; }

	private:
		const int				mWhat;
		const std::string		mName;
		std::string				mChannel;
	};

private:
	std::map<int, std::string>	mMsgs;
	std::map<std::string, std::function<ds::Event*()>> mCreators;
};

} // namespace event


/**
 * DEPRECATED. Use the RegisteredEvent<> subclass instead.
 */

/**
 * \class ds::EventRegistry
 * Utility to make sure all event types are unique, and named.
 */
class EventRegistry {
public:
	static const std::string&	getName(const int what);

	EventRegistry(const std::string& name);
	// What is now obsolete, the value is generated automatically.
	// This only exists for backwards compatibility
	EventRegistry(const int what, const std::string& name);

	const int				mWhat;
};

} // namespace ds

#endif // DS_APP_EVENT_H_
