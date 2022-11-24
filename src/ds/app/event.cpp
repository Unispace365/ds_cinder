#include "stdafx.h"

#include <ds/app/event.h>

#include <Poco/Mutex.h>
#include <assert.h>
#include <ds/debug/debug_defines.h>
#include <unordered_map>

#ifndef _WIN32
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#endif

namespace ds {

const std::string demangleTypeName(const std::string& n) {
#ifndef _WIN32
	// Non VisualStudio name demangler, uses C++11
	// From stackoverflow: 281818
	int									   status = -1;
	std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(n.c_str(), NULL, NULL, &status), std::free};

	const auto name = ((status == 0) ? res.get() : n);
#else
	const auto name = n;
#endif

	// Return only the part of the name after the last colon
	return name.substr(name.find_last_of(':') + 1);
}

/**
 * \class Event
 */
Event::Event()
  : mWhat(0)
  , mEventOrigin(0.0f, 0.0f, 0.0f)
  , mUserStringData("")
  , mUserId(0)
  , mUserSize(0.0f, 0.0f, 0.0f)
  , mSpriteOriginator(nullptr) {}

Event::Event(const size_t what)
  : mWhat(what)
  , mEventOrigin(0.0f, 0.0f, 0.0f)
  , mUserStringData("")
  , mUserId(0)
  , mUserSize(0.0f, 0.0f, 0.0f)
  , mSpriteOriginator(nullptr) {}

const std::string Event::getName() const {
	auto theName = EventRegistry::getName(mWhat);
	if (theName.empty()) theName = mEventName;
	return theName;
}

Event::~Event() {}

} // namespace ds
