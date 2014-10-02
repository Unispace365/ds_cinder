#pragma once
#ifndef DS_APP_ENGINE_ENGINEIODEFS_H_
#define DS_APP_ENGINE_ENGINEIODEFS_H_

#include <string>
#include "ds/app/app_defs.h"

namespace ds {
class DataBuffer;

// Server -> Client communication
extern const char				CMD_SERVER_SEND_WORLD;		// The server is sending the entire world
extern const char				CMD_CLIENT_STARTED_REPLY;	// The server has received a CLIENT_STARTED, and
															// is supplying the client with a session ID.

// Client -> Server communication
extern const char				CMD_CLIENT_STARTED;			// The client is notifying the server it's started,
															// and is supplying the server with a global ID.

extern const char				CMD_CLIENT_REQUEST_WORLD;	// The client is requesting the entire world

extern const char				CMD_CLIENT_RUNNING;			// A general heartbeat from the client.

// ATTRIBUTES
extern const char				ATT_CLIENT;					// Header for a client, which might have: ATT_GLOBAL_ID, ATT_SESSION_ID
extern const char				ATT_GLOBAL_ID;				// A string, which is a GUID
extern const char				ATT_SESSION_ID;				// An int32, which is a client-unique ID
extern const char				ATT_FRAME;					// A frame number

/**
 * \class ds::EngineIoInfo
 * \brief Store information related to doing IO.
 */
class EngineIoInfo {
public:
	EngineIoInfo();

	// Clients create a GUID on startup, which gets converted to a
	// much shorter sessionID by the server.
	std::string					mGlobalId;
};

/**
 * \class ds::ScopedClientAtts
 * \brief Used during Sprite::writeClientAttributesTo() to add the
 * header and footer boilerplate to the buffer.
 */
class ScopedClientAtts {
public:
	ScopedClientAtts(ds::DataBuffer&, const sprite_id_t);
	~ScopedClientAtts();

private:
	ds::DataBuffer&				mBuffer;
};

} // namespace ds

#endif