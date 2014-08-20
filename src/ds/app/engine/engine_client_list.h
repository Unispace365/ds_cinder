#pragma once
#ifndef DS_APP_ENGINE_ENGINECLIENTLIST_H_
#define DS_APP_ENGINE_ENGINECLIENTLIST_H_

#include <stdint.h>
#include <string>
#include <vector>

namespace ds {

/**
 * \class ds::EngineClientList
 * \brief List of currently-registered clients.
 */
class EngineClientList {
public:
	class State {
	public:
		State();
		State(const std::string &guid, const int32_t sessionid);

		std::string				mGuid;
		int32_t					mSessionId;
	};

public:
	EngineClientList();

	// Answer the new client ID, or < 1 for invalid
	int32_t						startClient(const std::string &guid);
	const State*				findClient(const int32_t) const;

private:
	std::vector<State>			mClients;

	// Track the next session ID to use
	int32_t						mNextSessionId;
};

} // namespace ds

#endif