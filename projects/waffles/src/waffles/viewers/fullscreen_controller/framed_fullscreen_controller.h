#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/app/event_client.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/media/media_interface.h>
#include <ds/ui/sprite/text.h>

namespace waffles {
class DrawingTools;

/**
 * \class ds::FramedFullscreenController
 *			A viewer panel that controls fullscreen assets
 */
class FramedFullscreenController : public BaseElement {
  public:
	FramedFullscreenController(ds::ui::SpriteEngine& g,
							   const std::string	 layout = "waffles/viewer/fullscreen_controller.xml");

	void linkMediaViewer(TitledMediaViewer* tmv);
	void collapseAndMove(ci::vec3 pos);
	void uncollapseAndMove(ci::vec3 pos);
	bool mIsCollapsed = false;

  protected:
	virtual void onLayout();
	void		 updateUi();
	void		 updateLockedState();
	void		 setDrawingToolsState();
	void		 removeDrawingTools();
	void		 setKeyboardButtonImage(std::string imageFile, ds::ui::ImageButton* keyboardBtn);
	void		 collapse();
	void		 uncollapse();
	void		 onUpdateServer(const ds::UpdateParams& p) override;
	void	init();
	virtual void onAboutToBeRemoved();
	virtual void onParentSet();


	ds::ui::SmartLayout*	mRootLayout		= nullptr;
	ds::ui::MediaInterface* mMediaInterface = nullptr;
	DrawingTools*			mDrawingTools	= nullptr;
	bool					mInitalizeded	= false;

	TitledMediaViewer* mLinkedMediaViewer = nullptr;
	std::string		   mLayoutFile		  = "waffles/viewer/fullscreen_controller.xml";

	bool	 mUncollapsedSizeSet = false;
	ci::vec3 mUncollapsedSize;
};

} // namespace waffles
