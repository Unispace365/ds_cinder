#pragma once

#include <ds/ui/layout/smart_layout.h>
#include <waffles/waffles_events.h>
#include <ds/app/engine/engine_events.h>

namespace ds::ui {
class TouchMenu;
}

namespace waffles {
class ViewerController;
class BackgroundView;
}

namespace downstream {

/**
 * \class waffles::AssetLayer
 *			The Asset layer. Holds Waffles
 */
template <class VC=waffles::ViewerController>
class WafflesSprite : public ds::ui::SmartLayout {
  public:
	WafflesSprite(ds::ui::SpriteEngine& eng);

	virtual void onSizeChanged() override;
	
	/**
	 * \brief called when onShow event is triggered
	 *		The default implementation of this method looks calls 
	 * \param e The idle started event
	 */
	virtual void onShow(const waffles::ShowWaffles& e);
	/**
	 * \brief called when onHide event is triggered
	 *		The default implementation of this method ???
	 * \param e The idle started event
	 */
	virtual void onHide(const waffles::HideWaffles& e);
	/**
	* \brief called when onIdleStarted event is triggered
	*		The default implementation of this method triggers a HideWaffles event
	* \param e The idle started event
	*/
	virtual void onIdleStarted(const ds::app::IdleStartedEvent& e);
	virtual VC* getViewerController() { return mViewerController; }
  private:
  	void setupTouchMenu();

	VC* mViewerController = nullptr;

	ds::ui::TouchMenu* mTouchMenu = nullptr;
	


};

} // namespace waffles
