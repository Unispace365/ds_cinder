#include "stdafx.h"

#include "idle_timer.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {

IdleTimer::IdleTimer(ui::SpriteEngine &engine)
	: mEngine(engine)
	, mActive(false)
	, mSetup(false)
	, mIdling(false)
{

}

void IdleTimer::setSecondBeforeIdle(const double idleTime)
{
	mSetup = true;
	mActive = true;
	mIdleTime = idleTime;
	resetIdleTimer();
}

double IdleTimer::secondsToIdle() const
{
	if(!mActive || !mSetup)
		return 0.0;

	return mIdleTime - (mEngine.getElapsedTimeSeconds() - mStartTime);
}

bool IdleTimer::isIdling() const
{
	if(!mActive || !mSetup)
		return false;

	return mIdling;
}

void IdleTimer::startIdling()
{
	if(!mSetup)
		return;

	mIdling = true;
}

void IdleTimer::resetIdleTimer()
{
	if(!mSetup)
		return;

	mIdling = false;
	mStartTime = mEngine.getElapsedTimeSeconds();
}

void IdleTimer::update()
{
	if(!mActive || !mSetup)
		return;

	if(!mIdling)
		mIdling = (mEngine.getElapsedTimeSeconds() - mStartTime) > mIdleTime;
}

void IdleTimer::clear()
{
	mActive = false;
	mSetup = false;
}

}