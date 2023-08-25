#pragma once
#ifndef DS_UTIL_IDLE_TIMER_H
#define DS_UTIL_IDLE_TIMER_H

namespace ds {
namespace ui {
	class SpriteEngine;
}

class IdleTimer {
  public:
	IdleTimer(ui::SpriteEngine& engine);
	void   setSecondBeforeIdle(const double);
	double secondsToIdle() const;
	bool   isIdling() const;
	void   startIdling();
	void   resetIdleTimer();
	void   clear();
	void   update();

  private:
	ui::SpriteEngine& mEngine;
	double			  mStartTime;
	double			  mIdleTime;
	bool			  mActive;
	bool			  mSetup;
	bool			  mIdling;
};

} // namespace ds

#endif // DS_UTIL_IDLE_TIMER_H
