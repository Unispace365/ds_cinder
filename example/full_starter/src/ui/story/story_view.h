#pragma once
#ifndef _FULLSTARTER_UI_STORY_STORY_VIEW_H_
#define _FULLSTARTER_UI_STORY_STORY_VIEW_H_


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>

#include "model/generated/story_model.h"

namespace fullstarter {

class Globals;

/**
* \class fullstarter::StoryView
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

	ds::ui::Text*						mMessage;

};

} // namespace fullstarter

#endif
