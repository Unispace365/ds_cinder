#pragma once
#ifndef DS_UTIL_DATEUTIL_H_
#define DS_UTIL_DATEUTIL_H_

#include <Poco/DateTime.h>

namespace ds {

	bool sameDay(const Poco::DateTime& a, const Poco::DateTime& b);
	bool sameWeek(const Poco::DateTime& a, const Poco::DateTime& b);
	bool sameMonth(const Poco::DateTime& a, const Poco::DateTime& b);

} // namespace ds

#endif // DS_UTIL_COLORUTIL_H_
