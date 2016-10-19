#pragma once
#ifndef _PANGO_APP_UI_STORY_STORY_VIEW_H_
#define _PANGO_APP_UI_STORY_STORY_VIEW_H_


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/image.h>

#include "model/generated/story_model.h"

#include "ds/ui/sprite/text_pango.h"

namespace pango {

class Globals;

/**
* \class pango::StoryView
*			A sample view
*/
class StoryView final : public ds::ui::Sprite  {
public:
	StoryView(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	virtual void						updateServer(const ds::UpdateParams& p);
	virtual void						drawLocalClient();

	void								animateOn();
	void								animateOff();

	void								setData();

	void								layout();

	Globals&							mGlobals;

	ds::EventClient						mEventClient;
	ds::ui::MultilineText*				mMessage;
	ds::ui::Image*						mImage;
	ds::ui::TextPango*					mPangoText;
	ds::ui::Sprite*						mFakeCursor;

};

} // namespace pango

#endif
