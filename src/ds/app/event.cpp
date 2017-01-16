#include "stdafx.h"

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
	, mEventOrigin(0.0f, 0.0f, 0.0f)
	, mUserStringData("")
	, mUserId(0)
	, mUserSize(0.0f, 0.0f, 0.0f)
	, mSpriteOriginator(nullptr)
{
}

Event::Event(const int what)
	: mWhat(what)
	, mEventOrigin(0.0f, 0.0f, 0.0f)
	, mUserStringData("")
	, mUserId(0)
	, mUserSize(0.0f, 0.0f, 0.0f)
	, mSpriteOriginator(nullptr)
{
}

const std::string& Event::getName() const {
	return EventRegistry::getName(mWhat);
}

Event::~Event() {
}

} // namespace ds
