#include "na/app/event.h"

#include <assert.h>
#include <unordered_map>
#include <Poco/Mutex.h>
#include <framework/debug/debug_ds.h>

namespace na {

namespace {
const std::string                             EMPTY_SZ("");

// Currently, we are requiring lock protection on the event map,
// although really, this stuff should always happen during initialization
static Poco::Mutex                            REGISTER_LOCK;
static std::unordered_map<int, std::string>   EVENTS;

static void register_event(const int what, const std::string& name)
{
  Poco::Mutex::ScopedLock   l(REGISTER_LOCK);

  if (!EVENTS.empty()) {
    auto f = EVENTS.find(what);
    if (f != EVENTS.end()) {
      DS_DBG_CODE(std::cout << "ERROR Event::registerEvent() ALREADY REGISTERED " << what << " (requested " << name << ", have " << f->second << ")" << std::endl);
      assert(false);
      return;
    }
  }
  EVENTS[what] = name;
}

}

/**
 * \class na::Event
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
  Poco::Mutex::ScopedLock   l(REGISTER_LOCK);
  if (EVENTS.empty()) return EMPTY_SZ;
  auto f = EVENTS.find(mWhat);
  if (f != EVENTS.end()) return f->second;
  return EMPTY_SZ;
}

Event::~Event()
{
}

/**
 * \class na::EventRegistry
 */
EventRegistry::EventRegistry(const int what, const std::string& name)
  : mWhat(what)
{
  register_event(what, name);
}

} // namespace na
