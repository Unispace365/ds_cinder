#ifndef DS_ESSENTIALS__TOUCH_VIEWDRAGGER_H_
#define DS_ESSENTIALS__TOUCH_VIEWDRAGGER_H_

#include "ds/ui/touch/momentum.h"
#include <ds/ui/sprite/sprite.h>

namespace ds {

/**
 * \class ViewDragger
 * Utility class that just handles dragging a view around with momentum.
 * Takes over any touch handling from parent.
 */
class ViewDragger : ds::AutoUpdate {
  public:
	ViewDragger(ds::ui::Sprite& parent);

	/** Automatically updated from auto update. */
	virtual void update(const ds::UpdateParams&);

	/** If there are fingers currently touching the parent. */
	bool hasTouches() const;

	/** The amount of time it takes to animate the parent back into the bounding box, in seconds. */
	void setReturnTime(const float newReturnTime) { mReturnTime = newReturnTime; }

	/** The rectangle to keep the parent sprite inside of. Defaults to the size of the world. */
	void setBoundingArea(const ci::Rectf& boundingArea) { mBoundingArea = boundingArea; }

	/** Check if the parent resides inside the Bounding Area.
		\param immediate If true, will set the position,
				if false, will animate back into the bounding box using return time for the duration.*/
	void checkBounds(const bool immediate = false);

  private:
	void onTouched(const ds::ui::TouchInfo&);

	ds::ui::Sprite& mParent;
	ds::Momentum	mMomentum;
	// STATE
	bool mIsTouchy;
	// SETTINGS
	float	  mReturnTime;
	ci::Rectf mBoundingArea;
};

} // namespace ds

#endif //! DS_ESSENTIALS__TOUCH_VIEWDRAGGER_H_