#pragma once
#ifndef DS_TIME_TIMER_H_
#define DS_TIME_TIMER_H_

#include <Poco/Timestamp.h>

namespace ds {
namespace time {

/**
 * \class ds::time::Timer
 * \brief Basic timer. I discovered at home that boost::timer is
 * completely non-portable across OSs (in terms of timing seconds;
 * it seems to be timing an abstract notion of CPU time), so use this.
 */
class Timer {
public:

	Timer();
	Timer(bool start);

	void						restart();
	// Restart with the supplied offset. If this value is positive,
	// you will get negative elapsed values.
	void						stop();
	void						restart(const double offset);
	double						elapsed();
	
private:
	Poco::Timestamp::TimeVal	mStart;
	bool						mIsStarted;
};

} // namespace time
} // namespace ds

#endif // DS_TIME_TIMER_H_