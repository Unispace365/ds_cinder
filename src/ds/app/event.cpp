#include <ds/app/event.h>

#include <assert.h>
#include <unordered_map>
#include <Poco/Mutex.h>
#include <ds/debug/debug_defines.h>

namespace ds {

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

const std::string& Event::getName() const {
	return EventRegistry::getName(mWhat);
}

Event::~Event() {
}

} // namespace ds
