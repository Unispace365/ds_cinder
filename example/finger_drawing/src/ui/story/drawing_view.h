#pragma once
#ifndef _FINGER_DRAWING_APP_UI_STORY_STORY_VIEW_H_
#define _FINGER_DRAWING_APP_UI_STORY_STORY_VIEW_H_


#include <ds/app/event_client.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/drawing/drawing_canvas.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/text.h>

#include "model/generated/story_model.h"

namespace example {

class Globals;

/**
 * \class example::StoryView
 *			A sample view
 */
class DrawingView final : public ds::ui::Sprite {
  public:
	DrawingView(Globals& g);

  private:
	void onAppEvent(const ds::Event&);

	virtual void onUpdateServer(const ds::UpdateParams& p) override;

	void animateOn();
	void animateOff();

	void				  layout();
	ds::ui::LayoutButton* configureBrushColorButton(const std::string&					   buttonName,
													std::map<std::string, ds::ui::Sprite*> spriteMap,
													const bool isBrush, const bool isErase = false);
	ds::ui::LayoutButton* configureBrushSizeButton(const std::string&					  buttonName,
												   std::map<std::string, ds::ui::Sprite*> spriteMap,
												   const float							  brushSize);

	Globals& mGlobals;

	ds::EventClient		   mEventClient;
	ds::ui::LayoutSprite*  mPrimaryLayout;
	ds::ui::Text*		   mMessage;
	ds::ui::Sprite*		   mBackground;
	ds::ui::Sprite*		   mDrawingHolder;
	ds::ui::DrawingCanvas* mDrawingCanvas;

	std::vector<ds::ui::LayoutButton*> mBrushColorButtons;
	std::vector<ds::ui::LayoutButton*> mBackgroundColorButtons;
	std::vector<ds::ui::LayoutButton*> mSizeButtons;
};

} // namespace example

#endif
