#include "stdafx.h"

#include <ds/app/event_registry.h>

#include <assert.h>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <Poco/Mutex.h>
#include <ds/debug/debug_defines.h>
#include <ds/debug/logger.h>
#include "ds/app/event.h"

namespace {

const std::string& getEmptySz() {
	static const std::string EMPTY_SZ("");
	return EMPTY_SZ;
}

static ds::Event BLANKEVENT = ds::Event();
const std::function<ds::Event*()> getEmptyCreator() {
	static std::function<ds::Event*()> func = []()->ds::Event*{ return &BLANKEVENT; };
	return func;
}
}

namespace ds {

namespace event {



/**
 * \class Registry
 */
Registry& Registry::get() {
	static Registry	REGISTRY;
	return REGISTRY;
}

Registry::Registry() {
}

size_t Registry::add(const std::string &name) {
	// For backwards compatibility with the old registry. Please tell
	// me no one has created this many old-style events.
	static const int		START(10000);
	size_t					ans = mMsgs.size() + 1 + START;
	mMsgs[ans] = name;
	return ans;
}

void Registry::report() {
	std::cout << "rf::msg::Registry size=" << mMsgs.size() << std::endl;
	for (auto it = mMsgs.begin(), end = mMsgs.end(); it != end; ++it) {
		std::cout << "\t" << it->first << ": " << it->second << std::endl;
	}
}

const std::string& Registry::getName(const size_t what){
	auto f = mMsgs.find(what);
	if(f != mMsgs.end()){
		return f->second;
	}

	return getEmptySz();
}

void Registry::addEventCreator(const std::string& eventName, std::function<ds::Event*()> creator){
	mCreators[eventName] = creator;
}
std::function<ds::Event*()>  Registry::getEventCreator(const std::string& eventName){
	auto f = mCreators.find(eventName);
	if(f != mCreators.end()){
		return f->second;
	}

	DS_LOG_WARNING("No auto-creator found for event name " << eventName);
	return getEmptyCreator();
}

/**
 * \class Entry
 */
Registry::Entry::Entry(const std::string &name)
		: mWhat(Registry::get().add(name))
		, mName(name) {
	std::stringstream		buf;
	buf << "event/" << mWhat;
	mChannel = buf.str();
}

} // namespace event


/**
 * DEPRECATED. Use the RegisteredEvent<> subclass instead.
 */

namespace {


// Currently, we are requiring lock protection on the event map,
// although really, this stuff should always happen during initialization

static Poco::Mutex& get_register_lock() {
	static Poco::Mutex  REGISTER_LOCK;
	return REGISTER_LOCK;
}

static std::unordered_map<size_t, std::string>& get_events() {
	static std::unordered_map<size_t, std::string>   EVENTS;
	return EVENTS;
}

// Old-style, where clients were supplying the what
static void register_event_deprecated(const size_t what, const std::string& name) {
	Poco::Mutex::ScopedLock   l(get_register_lock());
	auto& e = get_events();
	if (!e.empty()) {
		auto f = e.find(what);
		if (f != e.end()) {
			DS_LOG_VERBOSE(1, "ERROR Event::registerEvent() ALREADY REGISTERED " << what << " (requested " << name << ", have " << f->second << ")");
			//assert(false);
			return;
		}
	}
	e[what] = name;
}

static size_t register_event(const std::string& name) {

	Poco::Mutex::ScopedLock		l(get_register_lock());
	auto&						e = get_events();
	size_t						what = e.size() + 1;
	if (!e.empty()) {
		auto f = e.find(what);
		if (f != e.end()) {
			DS_LOG_VERBOSE(1, "ERROR Event::registerEvent() ALREADY REGISTERED " << what << " (requested " << name << ", have " << f->second << ")");
			//assert(false);
		}
	};
	e[what] = name;
	return what;
}

}

/**
 * \class EventRegistry
 */
const std::string& EventRegistry::getName(const size_t what) {
	Poco::Mutex::ScopedLock   l(get_register_lock());
	auto& e = get_events();
	if (e.empty()) return getEmptySz();
	auto f = e.find(what);
	if (f != e.end()) return f->second;
	return event::Registry::get().getName(what);
}

EventRegistry::EventRegistry(const std::string& name)
	: mWhat(register_event(name))
{
}

EventRegistry::EventRegistry(const size_t what, const std::string& name)
	: mWhat(register_event(name))
{
//	register_event(what, name);
}

} // namespace ds
