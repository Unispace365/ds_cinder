#pragma once

#include <ds/app/event.h>

namespace waffles {

class StateSaveRequest : public ds::RegisteredEvent<StateSaveRequest> {
  public:
	StateSaveRequest(const std::string& stateName)
		: mStateName(stateName) {}
	const std::string& mStateName;
};

class StateLoadRequest : public ds::RegisteredEvent<StateLoadRequest> {
  public:
	StateLoadRequest(const std::string& stateName)
		: mStateName(stateName) {}
	const std::string& mStateName;
};

class StateDeleteRequest : public ds::RegisteredEvent<StateDeleteRequest> {
  public:
	StateDeleteRequest(const std::string& stateName)
		: mStateName(stateName) {}
	const std::string& mStateName;
};

class StatesUpdatedEvent : public ds::RegisteredEvent<StatesUpdatedEvent> {
  public:
	StatesUpdatedEvent() {}
};

} // namespace waffles
