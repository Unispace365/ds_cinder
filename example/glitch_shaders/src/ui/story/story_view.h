#pragma once
#ifndef _GLITCH_SHADERS_APP_UI_STORY_STORY_VIEW_H_
#define _GLITCH_SHADERS_APP_UI_STORY_STORY_VIEW_H_


#include <ds/app/event_client.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/sprite/video.h>

#include "model/generated/story_model.h"

namespace downstream {

class Globals;

/**
 * \class downstream::StoryView
 *			A sample view
 */
class StoryView final : public ds::ui::Sprite {
  public:
	StoryView(Globals& g);

  private:
	void onAppEvent(const ds::Event&);

	virtual void onUpdateServer(const ds::UpdateParams& p) override;
	virtual void drawLocalClient() override;

	void animateOn();
	void animateOff();

	void setData();

	void	 layout();
	void	 incrementShader();
	Globals& mGlobals;

	ds::EventClient mEventClient;

	ds::ui::Video* mTheVideo;

	int mCurShader;
	int mNumShaders;
};

} // namespace downstream

#endif
