#include "ds/time/timer.h"

namespace ds {
namespace time {

namespace {
const double			RESOLUTION_D = static_cast<double>(Poco::Timestamp::resolution());
}

/**
 * \class ds::time::Timer
 */
Timer::Timer() {
	restart();
}

void Timer::restart() {
	mStart = Poco::Timestamp().epochMicroseconds();
}

void Timer::restart(const double offset) {
	Poco::Timestamp::TimeVal toffset = static_cast<Poco::Timestamp::TimeVal>(offset * RESOLUTION_D);
	restart();
	mStart += toffset;
}

double Timer::elapsed() {
	Poco::Timestamp::TimeVal	tv = Poco::Timestamp().epochMicroseconds() - mStart;
	return static_cast<double>(tv) / RESOLUTION_D;
}

} // namespace time
} // namespace ds