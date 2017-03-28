#include "stdafx.h"

#include "date_util.h"

namespace ds {

	bool sameDay(const Poco::DateTime& a, const Poco::DateTime& b){
		return(a.dayOfYear() == b.dayOfYear() && a.year() == b.year());
	}

	bool sameWeek(const Poco::DateTime& a, const Poco::DateTime& b){
		return(a.week(0) == b.week() && a.year() == b.year());
	}

	bool sameMonth(const Poco::DateTime& a, const Poco::DateTime& b){
		return(a.month() == b.month() && a.year() == b.year());
	}

} // namespace ds