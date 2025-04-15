#pragma once


#include <ds/ui/button/button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/layout_sprite.h>

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

		LayoutButton* setUncheckedButton(LayoutButton* unchecked);
		LayoutButton* setCheckedButton(LayoutButton* checked);

		LayoutButton* getUncheckedButton() const { return mUnchecked; }
		LayoutButton* getCheckedButton() const { return mChecked; }

		void setNormalButtonColor(const ci::ColorA& color) const {
			if (mUnchecked) mUnchecked->setNormalSpriteColor(color);
			if (mChecked) mChecked->setNormalSpriteColor(color);
		}
		void setHighButtonColor(const ci::ColorA& color) const {
			if (mUnchecked) mUnchecked->setHighSpriteColor(color);
			if (mChecked) mChecked->setHighSpriteColor(color);
		}

		/// Provides backward compatibility with the old image button
		void setNormalImageColor(const ci::ColorA& color) const { setNormalButtonColor(color); }
		/// Provides backward compatibility with the old image button
		void setHighImageColor(const ci::ColorA& color) const { setHighButtonColor(color); }

		void setChecked(const bool checked);

		bool getChecked() const { return mIsChecked; }

	  private:
		/// VIEW
		LayoutButton* mChecked;
		LayoutButton* mUnchecked;

		bool mIsChecked = false;
	};

}} // namespace ds::ui
