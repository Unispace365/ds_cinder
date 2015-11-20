#pragma once
#ifndef _KEYBOARDEXAMPLE_APP_UI_STORY_STORY_VIEW_H_
#define _KEYBOARDEXAMPLE_APP_UI_STORY_STORY_VIEW_H_


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/multiline_text.h>

#include "model/generated/story_model.h"

namespace example {

class Globals;

/**
* \class example::StoryView
*			A sample view
*/
class StoryView final : public ds::ui::Sprite  {
public:
	StoryView(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	virtual void						updateServer(const ds::UpdateParams& p);

	void								animateOn();
	void								animateOff();

	void								setData();

	void								layout();

	typedef ds::ui::Sprite				inherited;
	Globals&							mGlobals;

	ds::EventClient						mEventClient;

	ds::ui::MultilineText*				mMessage;

};

} // namespace example

#endif
