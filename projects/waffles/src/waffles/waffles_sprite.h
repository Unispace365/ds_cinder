#pragma once

#include <ds/ui/layout/smart_layout.h>
#include <waffles/waffles_events.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/content/content_model.h>
#include "waffles/util/shadow_layout.h"
#include "waffles/pinboard/pinboard_button.h"
#include "waffles/util/capture_player.h"
#include "waffles/background/background_view.h"
#include "waffles/layer/template_layer.h"
#include "waffles/service/settings_service.h"
#include "waffles/service/drawing_upload_service.h"
#include "app/waffles_app_defs.h"



namespace ds::ui {
class TouchMenu;
}

namespace waffles {
class ViewerController;
class WafflesController;
class BackgroundView;
class TemplateConfig;

/**
 * \class waffles::AssetLayer
 *			The Asset layer. Holds Waffles
 */
//template <class VC=waffles::ViewerController>
class WafflesSprite : public ds::ui::SmartLayout {
  public:
	template <class VC = waffles::ViewerController>
	WafflesSprite(ds::ui::SpriteEngine& eng)
	  : ds::ui::SmartLayout(eng, "waffles/waffles_sprite.xml"), mSettingsService(eng) {
		static_assert(std::is_base_of_v<waffles::ViewerController, VC>,
					  "VC must be a subclass of waffles::ViewerController");
		mViewerController = new VC(mEngine, ci::vec2(getSize()));

		auto sh = new waffles::ShadowLayout(eng);
		sh->release();
		auto pb = new waffles::PinboardButton(eng, "waffles/pinboard/pinboard_button.xml");
		pb->release();
		auto cap = new waffles::CapturePlayer(eng);
		cap->release();

		initializeWaffles();
	}

	~WafflesSprite() {
		if (mDefaultWaffles == this) {
			mDefaultWaffles = nullptr;
		}
		if(mTemplateLayer->getParent() == nullptr){
			mTemplateLayer->release();
		}
		if(mBackgroundView->getParent() == nullptr){
			mBackgroundView->release();
		}
		if(mViewerController->getParent() == nullptr){
			mViewerController->release();
		}

	}
	static void setDefault(WafflesSprite* defaultWaffles) { mDefaultWaffles = defaultWaffles; }
	static WafflesSprite* getDefault() { return mDefaultWaffles; }

	virtual void initializeWaffles();
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
	static WafflesSprite* mDefaultWaffles;
	
	//set playlist uid?

  public:
	virtual waffles::ViewerController * getViewerController() { return mViewerController; }
	std::map<int, ci::vec2>			   mTouchPoints;
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

	waffles::SettingsService mSettingsService;
	waffles::ViewerController* mViewerController = nullptr;
	waffles::BackgroundView*   mBackgroundView = nullptr;
	waffles::TemplateLayer* mTemplateLayer = nullptr;


	ds::ui::TouchMenu* mTouchMenu = nullptr;
	//from old engage controller
	float					   mInteractiveDuration = 5.f;

	std::string mPlaylistUid;
	std::string mSlideUid;
	int			mPlaylistIdx = -1;

	ds::model::ContentModelRef mPlaylist;
	ds::model::ContentModelRef mSlide;
	TemplateConfig* mTemplateConfig = nullptr;

	waffles::DrawingUploadService* mDrawingUploadService=nullptr;

	bool mAmEngaged = false;

	

};

} // namespace waffles
