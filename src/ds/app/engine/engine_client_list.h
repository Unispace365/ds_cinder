#pragma once
#ifndef DS_APP_ENGINE_ENGINECLIENTLIST_H_
#define DS_APP_ENGINE_ENGINECLIENTLIST_H_

#include <cstdint>
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
		State(const std::string& guid, int32_t sessionid);

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
	void setErrorChannel(EventNotifier*);

	/// Answer the new client ID, or < 1 for invalid
	int32_t		 startClient(const std::string& guid);
	State*		 findClient(int32_t);
	const State* findClient(int32_t) const;

	void reportingIn(int32_t session_id, int32_t frame);

	void compare(int32_t server_frame);

  private:
	std::vector<State> mClients;		  //
	int32_t			   mNextSessionId;	  // Track the next session ID to use
	int32_t			   mDisconnectionLag; // Amount of frames a client can lag before I consider it disconnected
	EventNotifier*	   mErrorChannel;	  //
};

} // namespace ds

#endif
