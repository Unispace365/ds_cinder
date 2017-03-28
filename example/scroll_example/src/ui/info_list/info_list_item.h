#pragma once
#ifndef EXAMPLE_SCROLL_EXAMPLE_UI_INFO_LIST_INFO_LIST_ITEM
#define EXAMPLE_SCROLL_EXAMPLE_UI_INFO_LIST_INFO_LIST_ITEM

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/image.h>
#include "model/generated/story_model.h"

namespace example
{
	class Globals;

	// For simplicity, we're just using a wstring for the model
	// In a real app, you'd want to pass a full model class to store
	class InfoListItem : public ds::ui::Sprite {
	public:
		InfoListItem(Globals& g, const float widthy, const float heighty);

		ds::model::StoryRef			getInfo();
		void						setInfo(const ds::model::StoryRef model);

		void						animateOn(const float delay);

		// Highlighted or not
		// 1 == highlighted, 0 = normal
		// TODO: make an enum or use a common one
		void						setState(const int buttonState);

	private:
		void						layout();
		ds::model::StoryRef			mInfoModel;
		Globals&					mGlobals;

		ds::ui::Image*				mThumbnail;
		ds::ui::Sprite*				mBackground;
		ds::ui::Text*				mLabel;

	};

}

#endif

