#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/app/event_client.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/media/media_interface.h>
#include <ds/ui/sprite/text.h>

namespace waffles {
class DrawingTools;

/**
 * \class ds::FullscreenController
 *			A viewer panel that controls fullscreen assets
 */
class FullscreenController : public BaseElement {
  public:
	FullscreenController(ds::ui::SpriteEngine& g);

	void linkMediaViewer(TitledMediaViewer* tmv);

  protected:
	virtual void onLayout();
	void		 updateUi();
	void		 updateLockedState();
	void		 setDrawingToolsState();
	void		 removeDrawingTools();
	void		 setKeyboardButtonImage(std::string imageFile, ds::ui::ImageButton* keyboardBtn);
	void	init();
	virtual void onAboutToBeRemoved();
	virtual void onParentSet();

	ds::ui::SmartLayout*	mRootLayout;
	ds::ui::MediaInterface* mMediaInterface;
	DrawingTools*			mDrawingTools;
	bool mInitalizeded = false;

	TitledMediaViewer* mLinkedMediaViewer;
};

} // namespace waffles
