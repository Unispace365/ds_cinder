#include "stdafx.h"

#include "toggle_container.h"


#include <ds/app/environment.h>

#pragma warning(disable : 4355)

namespace ds { namespace ui {

	/**
	 * \class ToggleContainer
	 */
	ToggleContainer::ToggleContainer(SpriteEngine& eng, float width, float height)
	  : LayoutSprite(eng)
	  , mChecked(nullptr)
	  ,mUnchecked(nullptr) {

		setLayoutType(kLayoutNone);
		setSize(width, height);
	}

	LayoutButton* ToggleContainer::setCheckedButton(LayoutButton* checked) {
		if (mChecked == checked) return mChecked;

		auto		  visibility   = false;
		LayoutButton* returnButton = mChecked;
		if (returnButton) {
			removeChild(*returnButton);
			auto visibility = returnButton->visible();
		}
		mChecked = checked;
		if (mChecked) {
			if (visibility) {
				mChecked->show();
			} else {
				mChecked->hide();
			}
			addChildPtr(mChecked);
		}
		return returnButton;
	}

	LayoutButton* ToggleContainer::setUncheckedButton(LayoutButton* unchecked) {
		if (mUnchecked == unchecked) return mUnchecked;
		auto		  visibility   = true;
		LayoutButton* returnButton = mUnchecked;
		if (returnButton) {
			removeChild(*returnButton);
			auto visibility = returnButton->visible();
		}
		mUnchecked = unchecked;
		if (mUnchecked) {
			if (visibility) {
				mUnchecked->show();
			} else {
				mUnchecked->hide();
			}
			addChildPtr(mUnchecked);
		}
		return returnButton;
	}

	void ToggleContainer::setChecked(const bool checked) {
		mIsChecked = checked;
		if (mIsChecked) {
			mChecked->show();
			mUnchecked->hide();
		} else {
			mChecked->hide();
			mUnchecked->show();
		}
	}

}
} // namespace ds::ui
