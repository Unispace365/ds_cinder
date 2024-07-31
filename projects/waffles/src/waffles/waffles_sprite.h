#pragma once

#include <ds/ui/layout/smart_layout.h>
#include <waffles/waffles_events.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/content/content_model.h>
#include "waffles/util/shadow_layout.h"
namespace ds::ui {
class TouchMenu;
}

namespace waffles {
class ViewerController;
class WafflesController;
class BackgroundView;


/**
 * \class waffles::AssetLayer
 *			The Asset layer. Holds Waffles
 */
//template <class VC=waffles::ViewerController>
class WafflesSprite : public ds::ui::SmartLayout {
  public:
	template <class VC = waffles::ViewerController>
	WafflesSprite(ds::ui::SpriteEngine& eng)
	  : ds::ui::SmartLayout(eng, "waffles/waffles_sprite.xml") {
		static_assert(std::is_base_of_v<waffles::ViewerController, VC>,
					  "VC must be a subclass of waffles::ViewerController");
		if (auto holdy = getSprite("viewer_controller_holdy")) {
			mViewerController = new VC(mEngine, ci::vec2(getSize()));
			holdy->addChildPtr(mViewerController);
		}
		auto sh = new waffles::ShadowLayout(eng);
		sh->release();
		
		enable(false);

		runLayout();

		setupTouchMenu();

		listenToEvents<waffles::ShowWaffles>([this](const auto& ev) { onShow(ev); });

		listenToEvents<ds::app::IdleStartedEvent>([this](const auto& ev) { onIdleStarted(ev); });
		listenToEvents<waffles::HideWaffles>([this](const auto& ev) { onHide(ev); });
		listenToEvents<ds::app::IdleEndedEvent>([this](const auto& ev) { onIdleEnded(ev); });
		listenToEvents<ds::ScheduleUpdatedEvent>([this](const auto& ev) { onScheduleUpdated(ev); });
		listenToEvents<waffles::RequestPresentationEndEvent>([this](const auto& ev) { onPresentationEndRequest(ev); });
		listenToEvents<waffles::RequestEngagePresentation>([this](const auto& ev) { onPresentationStartRequest(ev); });
		listenToEvents<waffles::RequestEngageNext>([this](const auto& ev) { onNextRequest(ev); });
		listenToEvents<waffles::RequestEngageBack>([this](const auto& ev) { onBackRequest(ev); });
		listenToEvents<waffles::RequestPresentationAdvanceEvent>(
			[this](const auto& ev) { onPresentationAdvanceRequest(ev); });
		setDefaultPresentation();
	}

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
	virtual void onIdleEnded(const ds::app::IdleEndedEvent& e);

	//engagement events
	virtual void onScheduleUpdated(const ds::ScheduleUpdatedEvent& e);
	virtual void onPresentationEndRequest(const waffles::RequestPresentationEndEvent& e);
	virtual void onPresentationStartRequest(const waffles::RequestEngagePresentation& e);
	virtual void onNextRequest(const waffles::RequestEngageNext& e);
	virtual void onBackRequest(const waffles::RequestEngageBack& e);
	virtual void onPresentationAdvanceRequest(const waffles::RequestPresentationAdvanceEvent& e);
	
  protected:
	virtual void gotoItem(int index);
	//set playlist uid?

  public:
	virtual waffles::ViewerController * getViewerController() { return mViewerController; }
  private:
  	void setupTouchMenu();
	void setDefaultPresentation();
	void setPresentation(ds::model::ContentModelRef thePlaylist);
	bool setSlide(ds::model::ContentModelRef theSlide);
	void startPresentationMode();
	void endPresentationMode();
	void setData();

	void nextItem();

	void prevItem();


	waffles::ViewerController* mViewerController = nullptr;

	ds::ui::TouchMenu* mTouchMenu = nullptr;
	//from old engage controller
	float					   mInteractiveDuration = 5.f;

	std::string mPlaylistUid;
	std::string mSlideUid;
	int			mPlaylistIdx = -1;

	ds::model::ContentModelRef mPlaylist;
	ds::model::ContentModelRef mSlide;

	bool mAmEngaged = false;

};

} // namespace waffles
