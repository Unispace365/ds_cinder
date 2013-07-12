#include <ds/app/event.h>

#include <assert.h>
#include <unordered_map>
#include <Poco/Mutex.h>
#include <ds/debug/debug_defines.h>

namespace ds {

namespace {

const std::string& getEmptySz()
{
	static const std::string EMPTY_SZ("");
	return EMPTY_SZ;
}

// Currently, we are requiring lock protection on the event map,
// although really, this stuff should always happen during initialization

static Poco::Mutex& get_register_lock()
{
	static Poco::Mutex  REGISTER_LOCK;
	return REGISTER_LOCK;
}

static std::unordered_map<int, std::string>& get_events()
{
	static std::unordered_map<int, std::string>   EVENTS;
	return EVENTS;
}

// Old-style, where clients were supplying the what
static void register_event_deprecated(const int what, const std::string& name)
{
	Poco::Mutex::ScopedLock   l(get_register_lock());
	auto& e = get_events();
	if (!e.empty()) {
		auto f = e.find(what);
		if (f != e.end()) {
			DS_DBG_CODE(std::cout << "ERROR Event::registerEvent() ALREADY REGISTERED " << what << " (requested " << name << ", have " << f->second << ")" << std::endl);
			assert(false);
			return;
		}
	}
	e[what] = name;
}

static int register_event(const std::string& name)
{
	Poco::Mutex::ScopedLock		l(get_register_lock());
	auto&						e = get_events();
	int							what = e.size() + 1;
	e[what] = name;
	return what;
}

}

/**
 * \class ds::Event
 */
Event::Event()
	: mWhat(0)
{
}

Event::Event(const int what)
	: mWhat(what)
{
}

const std::string& Event::getName() const
{
	Poco::Mutex::ScopedLock   l(get_register_lock());
	auto& e = get_events();
	if (e.empty()) return getEmptySz();
	auto f = e.find(mWhat);
	if (f != e.end()) return f->second;
	return getEmptySz();
}

Event::~Event()
{
}

/**
 * \class ds::EventRegistry
 */
EventRegistry::EventRegistry(const std::string& name)
	: mWhat(register_event(name))
{
}

EventRegistry::EventRegistry(const int what, const std::string& name)
	: mWhat(register_event(name))
{
//	register_event(what, name);
}

} // namespace ds
