#pragma once
#ifndef EXXON_TOUCH_DELAYED_MOMENTUM_H_
#define EXXON_TOUCH_DELAYED_MOMENTUM_H_

#include <Poco/Timestamp.h>
#include <cinder/Vector.h>
#include <ds/app/auto_update.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <vector>

namespace ds { namespace ui {

	/**
	 * \class DelayedMomentum
	 * A helper class to have sprites move with momentum in 2d.
	 * This has more of a 'pull' feel than the regular momentum. This is intended for during the drag phase, and not so
	 * much after it. Currently single-direction only.
	 */

	class DelayedMomentum : public ds::AutoUpdate {
	  public:
		DelayedMomentum(ds::ui::SpriteEngine& enging, const float deltaBurndown = 0.95f,
						const int maxSmoothFrames = 20);

		// Removes all delta touches and current delta is set to 0.0
		void clear();

		// add a movement touch point
		void addDeltaPosition(const float delta);

		// get the most recent value
		const float getDelta();

		// Delta burn down is how quickly to fade off old inputs. Kind of like friction
		void		setDeltaBurndown(const float burndown) { mDeltaBurndown = burndown; }
		const float getDeltaBurndown() { return mDeltaBurndown; }

		// Smooth frames is the absolute number of inputs to keep.
		// Inputs after this amount get discarded
		void	  setNumSmoothFrames(const int smoothFrams) { mNumSmoothFrames = smoothFrams; }
		const int getNumSmoothFrames() { return mNumSmoothFrames; }

	  protected:
		virtual void update(const ds::UpdateParams&);

	  private:
		typedef ds::AutoUpdate inherited;


		std::vector<float> mDeltas;
		float			   mCurrentDelta;
		float			   mDeltaBurndown;
		int				   mNumSmoothFrames;
	};

}} // namespace ds::ui

#endif // EXXON_TOUCH_DELAYED_MOMENTUM_H_
