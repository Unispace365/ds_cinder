#pragma once
#ifndef DS_APP_ENGINE_ENGINECLIENTLIST_H_
#define DS_APP_ENGINE_ENGINECLIENTLIST_H_

#include <stdint.h>
#include <string>
#include <vector>

namespace ds {
class EventNotifier;

/**
 * \class EngineClientList
 * \brief List of currently-registered clients.
 */
class EngineClientList {
  public:
	class State {
	  public:
		State();
		State(const std::string& guid, const int32_t sessionid);

		std::string mGuid;
		int32_t		mSessionId;
		/// The last frame the client has received from the server,
		/// echoed back.
		int32_t mServerSentFrame;
		/// Error sent when we lose a connection
		std::string mConnectionError;
		/// Cache when my connection error is in the error list,
		/// so I can pull it out without going through all the message sending.
		bool mGlobalsHasConnectionError;
	};

  public:
	EngineClientList();

	/// Set the error channel
	void setErrorChannel(ds::EventNotifier*);

	/// Answer the new client ID, or < 1 for invalid
	int32_t		 startClient(const std::string& guid);
	State*		 findClient(const int32_t);
	const State* findClient(const int32_t) const;

	void reportingIn(const int32_t session_id, const int32_t frame);

	void compare(const int32_t server_frame);

  private:
	std::vector<State> mClients;

	/// Track the next session ID to use
	int32_t mNextSessionId;
	/// Amount of frames a client can lag before I consider it
	/// disconnected
	int32_t			   mDisconnectionLag;
	ds::EventNotifier* mErrorChannel;
};

} // namespace ds

#endif
