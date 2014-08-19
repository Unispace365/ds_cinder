#include "ds/app/engine/engine_client_list.h"

namespace ds {

/**
 * \class ds::EngineClientList
 */
EngineClientList::EngineClientList()
		: mNextSessionId(1) {
}

int32_t EngineClientList::startClient(const std::string &guid) {
	if (guid.empty()) return 0;
	try {
		for (auto it=mClients.begin(), end=mClients.end(); it!=end; ++it) {
			if (it->mGuid == guid) {
				return it->mSessionId;
			}
		}
		mClients.push_back(State(guid, mNextSessionId++));
		return mClients.back().mSessionId;
	} catch (std::exception const&) {
	}
	return 0;
}

const EngineClientList::State* EngineClientList::findClient(const int32_t id) const {
	for (auto it=mClients.begin(), end=mClients.end(); it!=end; ++it) {
		if (it->mSessionId == id) return &(*it);
	}
	return nullptr;
}


/**
 * \class ds::EngineClientList::State
 */
EngineClientList::State::State()
		: mSessionId(0) {
}

EngineClientList::State::State(const std::string &guid, const int32_t sessionid)
		: mGuid(guid)
		, mSessionId(sessionid) {
}

} // namespace ds
