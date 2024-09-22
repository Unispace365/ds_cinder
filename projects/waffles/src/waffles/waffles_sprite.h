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

#include "ds/app/event_client.h"


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

	  WafflesSprite(ds::ui::SpriteEngine& eng, std::string eventChannel = "");

	  ~WafflesSprite();
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
	virtual void setupTouchMenu();
	static WafflesSprite* mDefaultWaffles;
	ds::ui::TouchMenu* mTouchMenu = nullptr;
	ds::EventClient mChannelClient;
	
	//set playlist uid?

  public:
	virtual waffles::ViewerController* getViewerController() { return mViewerController; }
	std::map<int, ci::vec2>			   mTouchPoints;
  private:
  	
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
	int mTimedCallback = 0;
	
	

};

} // namespace waffles
