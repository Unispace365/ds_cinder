#pragma once


#include <ds/ui/button/button.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/button/layout_button.h>

namespace ds { namespace ui {

	/**
	 * \class ToggleContainer
	 *	A convenience class that's basically a SpriteButton that also can Layout like a LayoutSprite.
	 *	NOTE: changing the layout type of this sprite may make up/down weird
	 *	This sprite is set to layoutNone, which passes the runLayout() recursive call down, but doesn't affect the size
	 *or position of it's children The up/down sprites are set to fillsize and kLayoutSize, so changing the size of this
	 *(and running the layout) will make the up/down the same size and affect the size of it's children
	 */
	class ToggleContainer : public LayoutSprite {
	  public:
		ToggleContainer(SpriteEngine& eng, float width = 0.0f, float height = 0.0f);

		LayoutButton* setCheckedButton(LayoutButton* checked);

		LayoutButton* setUncheckedButton(LayoutButton* unchecked);

		LayoutButton* getUncheckedButton() const { return mUnchecked; }
		LayoutButton* getCheckedButton() const { return mChecked; }

		void setChecked(const bool checked);

		bool getChecked() const { return mIsChecked; }

	  private:
		

		/// VIEW
		LayoutButton* mChecked;
		LayoutButton* mUnchecked;

		bool mIsChecked = false;
		
	};

}} // namespace ds::ui

