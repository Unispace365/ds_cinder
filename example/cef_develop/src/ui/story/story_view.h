#pragma once
#ifndef _CEFDEVELOP_APP_UI_STORY_STORY_VIEW_H_
#define _CEFDEVELOP_APP_UI_STORY_STORY_VIEW_H_


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>

#include "model/generated/story_model.h"

namespace cef {

class Globals;

/**
* \class cef::StoryView
*			A sample view
*/
class StoryView final : public ds::ui::Sprite  {
public:
	StoryView(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	void								animateOn();
	void								animateOff();

	void								setData();

	void								layout();

	Globals&							mGlobals;

	ds::EventClient						mEventClient;

	ds::ui::Text*						mMessage;

};

} // namespace cef

#endif
