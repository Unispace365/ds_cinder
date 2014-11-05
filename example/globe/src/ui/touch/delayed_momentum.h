#pragma once
#ifndef DS_EXAMPLE_UI_TOUCH_DELAYED_MOMENTUM
#define DS_EXAMPLE_UI_TOUCH_DELAYED_MOMENTUM

#include <vector>
#include <cinder/Vector.h>
#include <Poco/Timestamp.h>
#include <ds/app/auto_update.h>


namespace globe_example {
class Globals;
/**
* \class globe_example
* A helper class to have sprites move with momentum in 2d.
* This has more of a 'pull' feel than the regular momentum. This is intended for during the drag phase, and not so much after it.
* Currently single-direction only.
*/

class DelayedMomentum final : public ds::AutoUpdate {
public:
	DelayedMomentum(Globals& globGlob);

	// Removes all delta touches and current delta is set to 0.0
	void						clear();

	// add a movement touch point
	void						addDeltaPosition(const float delta);

	// get the most recent value
	const float					getDelta();

protected:
	virtual void				update(const ds::UpdateParams&);

private:
	typedef ds::AutoUpdate		inherited;
	Globals&					mGlobals;


	std::vector<float>			mDeltas;
	float						mCurrentDelta;
	float						mDeltaBurndown;
	int							mNumSmoothFrames;

};

} // namespace ds

#endif // DS_EXAMPLE_UI_TOUCH_DELAYED_MOMENTUM