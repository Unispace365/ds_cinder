#pragma once
#ifndef DS_TIME_TIME_CALLBACK
#define DS_TIME_TIME_CALLBACK

#include <Poco/Timestamp.h>
#include <ds/app/auto_update.h>
#include <functional>

namespace ds {
namespace time {

/**
* \class Callback
* \brief Get a lambda function back after a certain amount of time. Only supports a single callback at a time
*/
class Callback : ds::AutoUpdate {
public:

	/// Construct a blank callback, use singleCallback() or repeatedCallback()
	Callback(ds::ui::SpriteEngine& eng);

	/// Repeatedly callback
	Callback(ds::ui::SpriteEngine& eng, std::function<void()> func, const double secondsDelay);

	Callback(Callback&& cb);
	
	/// Waits the specified amount of time, calls the function, then stops
	/// Returns an ID so you can cancel it later if you need to (if you're starting this from the engine)
	size_t timedCallback(std::function<void()> func, const double secondsCallback);

	/// Waits the amount of time, calls the function, then repeats again and again
	/// Returns an ID so you can cancel it later if you need to (if you're starting this from the engine)
	size_t repeatedCallback(std::function<void()> func, const double secondsCallback);

	/// Cancels any upcoming callbacks, single or repeated
	void cancel();

	/// Get the Id for this instance
	size_t getId() { return mId; }

private:

	virtual void				update(const ds::UpdateParams& p) override;
	std::function<void()>		mCallback;
	Poco::Timestamp::TimeVal	mStart;
	size_t						mId;
	double						mDelay;
	bool						mRepeated;
	bool						mRunning;
};

} // namespace time
} // namespace ds

#endif // DS_TIME_TIME_CALLBACK
