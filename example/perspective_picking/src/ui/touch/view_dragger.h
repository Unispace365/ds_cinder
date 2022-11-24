#ifndef _PERSPECTIVEPICKING_APP_UI_TOUCH_VIEWDRAGGER_H_
#define _PERSPECTIVEPICKING_APP_UI_TOUCH_VIEWDRAGGER_H_

#include "ds/ui/touch/momentum.h"
#include <ds/ui/sprite/sprite.h>

namespace perspective_picking {
class Globals;

/**
 * \class na::ViewDragger
 * \brief Utility class that just handles dragging a view around with momentum.
 * NOTE: Takes over any touch handling from parent, which is fine for how
 * it's currently being used, but this is a quick class.
 */
class ViewDragger {
  public:
	ViewDragger(Globals&, ds::ui::Sprite& parent);

	void updateServer();
	bool hasTouches() const;
	// When an owning view first appears, it should call this once
	// to verify the view is in bounds.
	void startup();

  private:
	void onTouched(const ds::ui::TouchInfo&);
	void checkBounds(const bool immediate = false);

	ds::ui::Sprite& mParent;
	ds::Momentum	mMomentum;
	// STATE
	bool mIsTouchy;
	// SETTINGS
	const float mReturnTime;
};

} // namespace perspective_picking

#endif //!_PERSPECTIVEPICKING_APP_UI_TOUCH_VIEWDRAGGER_H_
