#include "stdafx.h"

#include "ds/app/engine/engine_client.h"

// Turn off an unnecessary warning in the boost GUID
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ds/app/engine/engine_io_defs.h"
#include "ds/app/engine/unique_id.h"
#include "ds/data/data_buffer.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <cinder/Json.h>
#include <ds/app/environment.h>


namespace ds {

const char			CMD_SERVER_SEND_WORLD = 1;
const char			CMD_CLIENT_STARTED_REPLY = 2;
const char			CMD_CLIENT_STARTED = 3;
const char			CMD_CLIENT_REQUEST_WORLD = 4;
const char			CMD_CLIENT_RUNNING = 5;

const char			ATT_CLIENT = 1;
const char			ATT_GLOBAL_ID = 2;
const char			ATT_SESSION_ID = 3;
const char			ATT_FRAME = 4;
const char			ATT_ROOTS = 5;

/**
 * \class EngineIoInfo
 */
EngineIoInfo::EngineIoInfo(ds::Engine& engine) {
	if (engine.getEngineSettings().hasSetting("platform:guid"))
	{
		/*!
		 * \note (Sepehr Laal) I figured there's no advantage of having the server dispatching
		 * world events over multiple IP addresses / ports. After all, this is -
		 * called multi casting for a reason! All it takes to run multiple clients
		 * on the same machine is to set "platform:guid" text entry inside engine.xml.
		 * You should make sure that each client has a unique string as its ID.
		 * With this ID, engine can track disconnected clients / etc.
		  
		 * GN: What Sepehr means is he was trying to run multiple clients on the same machine, and the cache below would
		 *     create the same guid, so i think he tried to have multiple IP/addesses to address the multiple clients.
		 *	   That's not really true any more, and you can in fact run multiple clients on the same machine without setting the guid.
		 *	   However, there's some value to being able to specify an ID for each client, so this setting is left here.
		 */
		mGlobalId = engine.getEngineSettings().getString("platform:guid");
	}
	else
	{
		const std::string	GUID("guid");
		std::string			cache_path(ds::Environment::expand("%LOCAL%/cache/ds/engine.json"));
		if (Poco::File(cache_path).exists()) {
			ci::JsonTree	tree(ci::loadFile(cache_path));
			for (auto it = tree.begin(), end = tree.end(); it != end; ++it) {
				if (it->getKey() == GUID) {
					mGlobalId = it->getValue();
				}
			}
		}
		// Designed for multiple cached values eventually (though don't know what they'd be)
		if (!mGlobalId.empty()) return;

		Poco::Path					path(cache_path);
		path = path.parent();
		Poco::File(path).createDirectories();

		if (mGlobalId.empty()) {
			mGlobalId = get_unique_id();
		}

		ci::JsonTree				tree(ci::JsonTree::makeArray());
		tree.pushBack(ci::JsonTree(GUID, mGlobalId));
		tree.write(cache_path);
	}
}

/**
 * \class ScopedClientAtts
 */
ScopedClientAtts::ScopedClientAtts(ds::DataBuffer &b, const sprite_id_t id)
		: mBuffer(b) {
	mBuffer.add(EngineClient::getClientStatusBlob());
	mBuffer.add(id);
}

ScopedClientAtts::~ScopedClientAtts() {
	mBuffer.add(TERMINATOR_CHAR);
}

} // namespace ds
