//#include "stdafx.h"

// Turn off an unnecessary warning in the boost GUID
#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace ds {

std::string					get_unique_id() {
	std::stringstream		buf;
	boost::uuids::uuid		uuid = boost::uuids::random_generator()();
	// I stuff the machine name in there solely so it's easier to identify a
	// problem machine when this gets used in an error report. It's entirely
	// possible that in practice, the machine name is the only thing needed,
	// but we'll play it safe.
	buf << boost::asio::ip::host_name() << "-" << uuid;
	return buf.str();
}

} // namespace ds
