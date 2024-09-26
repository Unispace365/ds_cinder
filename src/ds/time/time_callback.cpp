#include "time_callback.h"
#include "stdafx.h"

#include <ds/ui/sprite/sprite_engine.h>

namespace ds { namespace time {


	Callback::Callback(ds::ui::SpriteEngine& eng)
	  : ds::AutoUpdate(eng, AutoUpdateType::SERVER | AutoUpdateType::CLIENT)
	  , mCallback(nullptr)
	  , mDelay(0.0)
	  , mRepeated(false)
	  , mRunning(false) {}

	Callback::Callback(ds::ui::SpriteEngine& eng, std::function<void()> func, const double secondsDelay)
	  : ds::AutoUpdate(eng, AutoUpdateType::SERVER | AutoUpdateType::CLIENT)
	  , mCallback(nullptr)
	  , mDelay(0.0)
	  , mRepeated(true)
	  , mRunning(false) {
		repeatedCallback(func, secondsDelay);
	}

	Callback::Callback(Callback&& cb)
	  : ds::AutoUpdate(cb.mEngine, AutoUpdateType::SERVER | AutoUpdateType::CLIENT)
	  , mCallback(cb.mCallback)
	  , mStart(cb.mStart)
	  , mId(cb.mId)
	  , mDelay(cb.mDelay)
	  , mRepeated(cb.mRepeated)
	  , mRunning(cb.mRunning) {}

	size_t Callback::timedCallback(std::function<void()> func, const double secondsCallback) {
		mId		  = mEngine.mCallbackId++;
		mRunning  = true;
		mRepeated = false;
		mCallback = func;
		mDelay	  = secondsCallback;
		mStart	  = Poco::Timestamp().epochMicroseconds();
		return mId;
	}

	size_t Callback::repeatedCallback(std::function<void()> func, const double secondsCallback) {
		mId		  = mEngine.mCallbackId++;
		mRunning  = true;
		mRepeated = true;
		mCallback = func;
		mDelay	  = secondsCallback;
		mStart	  = Poco::Timestamp().epochMicroseconds();
		return mId;
	}

	void Callback::cancel() {
		mRunning  = false;
		mCallback = nullptr;
	}

	void Callback::update(const ds::UpdateParams& p) {
		if (mRunning && mCallback) {
			if (double(Poco::Timestamp().epochMicroseconds() - mStart) / Poco::Timestamp::resolution() > mDelay) {
				mCallback();
				if (mRepeated) {
					mStart = Poco::Timestamp().epochMicroseconds();
				} else {
					mRunning = false;
				}
			}
		}
	}


}} // namespace ds::time
