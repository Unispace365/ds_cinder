#pragma once
#ifndef _PANGO_APP_UI_STORY_STORY_VIEW_H_
#define _PANGO_APP_UI_STORY_STORY_VIEW_H_


#include <ds/app/event_client.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

#include "model/generated/story_model.h"


namespace pango {

class Globals;

/**
 * \class pango::StoryView
 *			A sample view
 */
class StoryView : public ds::ui::Sprite {
  public:
	StoryView(Globals& g);

  private:
	void onAppEvent(const ds::Event&);

	virtual void onUpdateServer(const ds::UpdateParams& p) override;

	void animateOn();
	void animateOff();

	void setData();

	void	 layout();
	void	 randomizeText();
	Globals& mGlobals;

	ds::EventClient mEventClient;
	ds::ui::Text*	mMessage;
	ds::ui::Image*	mImage;
	ds::ui::Text*	mPangoText;
	std::wstring	mFullText;
	ds::ui::Sprite* mFakeCursor;
};

} // namespace pango

#endif
