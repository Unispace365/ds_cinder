#include "observer.h"
#include "logger.h"
#include "cinder\app\App.h"
#include "ds\util\string_util.h"
#include "cinder\Rand.h"

const ds::BitMask	ds::OBSERVER_LOG = ds::Logger::newModule("observer");

namespace ds {
	Observer::Observer() : mTweaker(nullptr) {}

	Observer::~Observer() {
		if (mTweaker)
		{
			// Cleaning up in case a Class is to be deleted before execution termination
			TwDeleteBar(getObserver());
		}
	}

	std::string Observer::observerHashGenerator() const {

		DS_LOG_WARNING_M("You are using the default observers' hash generator"
			<< std::endl
			<< "This is not reliable and you have to provide a unique hash generator for your class"
			<< std::endl
			<< "override: virtual std::string Observer::hashGenerator() in your class"
			<< std::endl,
			ds::OBSERVER_LOG);

		uint32_t a = static_cast<uint32_t>(ci::randInt());
		a = (a ^ 61) ^ (a >> 16);
		a = a + (a << 3);
		a = a ^ (a >> 4);
		a = a * 0x27d4eb2d;
		a = a ^ (a >> 15);
		return value_to_string(a);
	}

	void Observer::installObserver() {
		if (mTweaker) return;
		mTweaker = TwNewBar(observerHashGenerator().c_str());
	}

	TwBar* Observer::getObserver() {
		return mTweaker;
	}
}