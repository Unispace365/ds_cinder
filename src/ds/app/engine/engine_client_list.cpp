#include "ds/app/engine/engine_client_list.h"

#include <iostream>
#include <sstream>
#include <ds/app/event_notifier.h>
#include <ds/util/string_util.h>
#include <ds/debug/logger.h>

namespace ds {

/**
 * \class ds::EngineClientList
 */
EngineClientList::EngineClientList()
		: mNextSessionId(1)
		, mDisconnectionLag(180)
		, mErrorChannel(nullptr) {
}

void EngineClientList::setErrorChannel(ds::EventNotifier *n) {
	mErrorChannel = n;
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

EngineClientList::State* EngineClientList::findClient(const int32_t id) {
	for (auto it=mClients.begin(), end=mClients.end(); it!=end; ++it) {
		if (it->mSessionId == id) return &(*it);
	}
	return nullptr;
}

const EngineClientList::State* EngineClientList::findClient(const int32_t id) const {
	for (auto it=mClients.begin(), end=mClients.end(); it!=end; ++it) {
		if (it->mSessionId == id) return &(*it);
	}
	return nullptr;
}

void EngineClientList::reportingIn(const int32_t session_id, const int32_t frame) {
	State* s = findClient(session_id);
	if (s) s->mServerSentFrame = frame;
}

void EngineClientList::compare(const int32_t server_frame) {
	for (auto it=mClients.begin(), end=mClients.end(); it!=end; ++it) {
		bool				needs_connection_error = false;
		const int32_t cf = it->mServerSentFrame;
		if (server_frame < cf) {
			// Something is very wrong
		} else if ((server_frame - cf) > mDisconnectionLag) {
			DS_LOG_ERROR("Client " << it->mGuid << " connection appears lost (behind by " << (server_frame - cf) << " frames)");
			needs_connection_error = true;
		}
		if (mErrorChannel && !it->mConnectionError.empty() && it->mGlobalsHasConnectionError != needs_connection_error) {
			if (needs_connection_error) mErrorChannel->notify(AddErrorEvent(it->mConnectionError));
			else mErrorChannel->notify(RemoveErrorEvent(it->mConnectionError.getId()));
			it->mGlobalsHasConnectionError = needs_connection_error;
		}
	}
}

/**
 * \class ds::EngineClientList::State
 */
EngineClientList::State::State()
		: mSessionId(0)
		, mServerSentFrame(-1)
		, mGlobalsHasConnectionError(false) {
}

EngineClientList::State::State(const std::string &guid, const int32_t sessionid)
		: mGuid(guid)
		, mSessionId(sessionid)
		, mServerSentFrame(-1)
		, mGlobalsHasConnectionError(false) {
	std::wstringstream		buf;
	buf << "The server has lost the connection to client  " << ds::wstr_from_utf8(guid) << ".";
	mConnectionError = ErrorRef(ErrorRef::getNextId(), L"Client connection lost", buf.str());
}

} // namespace ds
