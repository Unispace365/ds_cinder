// Turn off an unnecessary warning in the boost GUID
#define _SCL_SECURE_NO_WARNINGS

#include "ds/app/engine/engine_io_defs.h"

#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

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

/**
 * \class ds::EngineIoInfo
 */
EngineIoInfo::EngineIoInfo() {
	const std::string	GUID("guid");
	std::string			cache_path(ds::Environment::expand("%LOCAL%/cache/ds/engine.json"));
	if (Poco::File(cache_path).exists()) {
		ci::JsonTree	tree(ci::loadFile(cache_path));
		for (auto it=tree.begin(), end=tree.end(); it!=end; ++it) {
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
		std::stringstream		buf;
		boost::uuids::uuid		uuid = boost::uuids::random_generator()();
		// I stuff the machine name in there solely so it's easier to identify a
		// problem machine when this gets used in an error report. It's entirely
		// possible that in practice, the machine name is the only thing needed,
		// but we'll play it safe.
		buf << boost::asio::ip::host_name() << "-" << buf << uuid;
		mGlobalId = buf.str();
	}

	ci::JsonTree				tree(ci::JsonTree::makeArray());
	tree.pushBack(ci::JsonTree(GUID, mGlobalId));
	tree.write(cache_path);
}

} // namespace ds