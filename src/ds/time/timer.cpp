#include "stdafx.h"

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

Timer::Timer(bool start) {
	mIsStarted = start;
	if(start == true){
		restart();
	}
}

void Timer::stop() {
	mIsStarted = false;
}

void Timer::restart() {
	mStart = Poco::Timestamp().epochMicroseconds();
	mIsStarted = true;
}

void Timer::restart(const double offset) {
	Poco::Timestamp::TimeVal toffset = static_cast<Poco::Timestamp::TimeVal>(offset * RESOLUTION_D);
	restart();
	mStart += toffset;
	mIsStarted = true;
}

double Timer::elapsed() {
	if(mIsStarted){
		Poco::Timestamp::TimeVal	tv = Poco::Timestamp().epochMicroseconds() - mStart;
		return static_cast<double>(tv) / RESOLUTION_D;
	} else {
		return 0.0;
	}
}

} // namespace time
} // namespace ds