#pragma once
#ifndef _SETTINGS_REWRITE_APP_UI_STORY_STORY_VIEW_H_
#define _SETTINGS_REWRITE_APP_UI_STORY_STORY_VIEW_H_


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/control/control_check_box.h>

#include "model/generated/story_model.h"

namespace downstream {

class Globals;

/**
* \class downstream::StoryView
*			A sample view
*/
class StoryView final : public ds::ui::Sprite  {
public:
	StoryView(Globals& g);

	bool								getIsEngineMode();
	bool								getIncludeComments();

private:
	void								onAppEvent(const ds::Event&);

	virtual void						onUpdateServer(const ds::UpdateParams& p) override;

	void								animateOn();
	void								animateOff();

	void								setData();

	void								layout();

	Globals&							mGlobals;

	ds::EventClient						mEventClient;
	ds::ui::LayoutSprite*				mPrimaryLayout;
	ds::ui::Text*						mMessage;
	ds::ui::Image*						mImage;
	ds::ui::ControlCheckBox*			mIsEngineCheckbox;
	ds::ui::ControlCheckBox*			mIncludeComments;

};

} // namespace downstream

#endif

