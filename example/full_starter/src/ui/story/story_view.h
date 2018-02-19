#pragma once
#ifndef _FULLSTARTER_APP_UI_STORY_STORY_VIEW_H_
#define _FULLSTARTER_APP_UI_STORY_STORY_VIEW_H_


#include <ds/ui/sprite/sprite.h>
#include <ds/ui/layout/smart_layout.h>

#include "model/generated/story_model.h"

namespace fullstarter {

class Globals;

/**
* \class fullstarter::StoryView
*			A sample view
*/
class StoryView : public ds::ui::SmartLayout  {
public:
	StoryView(Globals& g);

private:
	virtual void						onUpdateServer(const ds::UpdateParams& p) override;

	void								animateOn();
	void								animateOff();

	void								setData();

	Globals&							mGlobals;


};

} // namespace fullstarter

#endif
