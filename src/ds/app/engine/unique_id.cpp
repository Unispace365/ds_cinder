#include "stdafx.h"


#include <cinder/Rand.h>
#include <sstream>
#include <string>

namespace ds {

std::string get_unique_id() {
	std::stringstream buf;
	auto			  compName = getenv("COMPUTERNAME");
	ci::Rand::randSeed((uint32_t)std::time(0));
	buf << compName << "-" << ci::randInt();
	return buf.str();
}
} // namespace ds