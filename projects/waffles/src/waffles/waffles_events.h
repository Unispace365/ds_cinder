#pragma once

#include <ds/app/event.h>
#include <ds/content/content_model.h>
#include "waffles/model/viewer_creation_args.h"

#include "waffles/viewers/viewer_controller.h"

namespace waffles {
class BaseElement;
class ScheduleHandler;

struct AmbientSettingsUpdated : public ds::RegisteredEvent<AmbientSettingsUpdated> {};

struct LockoutSettingsUpdated : public ds::RegisteredEvent<LockoutSettingsUpdated> {};

struct RequestViewerLaunchEvent : public ds::RegisteredEvent<RequestViewerLaunchEvent> {
	RequestViewerLaunchEvent(ViewerCreationArgs args)
		: mViewerArgs(args) {}
	ViewerCreationArgs mViewerArgs;
};

struct RequestCloseAllEvent : public ds::RegisteredEvent<RequestCloseAllEvent> {
	RequestCloseAllEvent(bool closeSlideContent = false) : mCloseSlideContent(closeSlideContent) {}
	RequestCloseAllEvent(const ci::vec3& location, bool closeSlideContent = false): mCloseSlideContent(closeSlideContent) { mEventOrigin = location; }

	bool mCloseSlideContent = false;
};

/// Request an arrangement of the current viewers.
struct RequestArrangeEvent : public ds::RegisteredEvent<RequestArrangeEvent> {
	RequestArrangeEvent() {}
	RequestArrangeEvent(const ci::vec3& location) { mEventOrigin = location; }
};

/// Requests all viewers to be gathered together
struct RequestGatherEvent : public ds::RegisteredEvent<RequestGatherEvent> {
	RequestGatherEvent(const ci::vec3& location) { mEventOrigin = location; }
};

struct WafflesLauncherOpened : public ds::RegisteredEvent<WafflesLauncherOpened> {};

struct ShowWaffles : public ds::RegisteredEvent<ShowWaffles> {};
struct HideWaffles : public ds::RegisteredEvent<HideWaffles> {};

struct CmsDataLoadCompleteEvent : public ds::RegisteredEvent<CmsDataLoadCompleteEvent> {};

struct SettingsInitializeComplete : public ds::RegisteredEvent<SettingsInitializeComplete> {};

struct RequestBackgroundChange : public ds::RegisteredEvent<RequestBackgroundChange> {
	RequestBackgroundChange(const int backgroundType, const ds::model::ContentModelRef media, const int pdfPage = 1)
		: mBackgroundType(backgroundType)
		, mMedia(media)
		, mPdfPage(pdfPage) {}
	const ds::model::ContentModelRef mMedia;
	const int						 mBackgroundType;
	const int						 mPdfPage;
};

// If there's a PDF in front, will advance through that, if there's no pdf, but there's a presentation, will go through
// that
struct RequestGenericAdvance : public ds::RegisteredEvent<RequestGenericAdvance> {
	RequestGenericAdvance(const bool goForwards)
		: mForwards(goForwards){};
	const bool mForwards;
};

// if a PDF is in front, advance or de-advance the page
struct RequestPDFPageChange : public ds::RegisteredEvent<RequestPDFPageChange> {
	RequestPDFPageChange(const bool goForwards)
		: mForwards(goForwards){};
	const bool mForwards;
};

// Goes forwards or back in the current presentation
struct RequestPresentationAdvanceEvent : public ds::RegisteredEvent<RequestPresentationAdvanceEvent> {
	RequestPresentationAdvanceEvent()
		: mForwards(true){};
	RequestPresentationAdvanceEvent(const bool goForwards)
		: mForwards(goForwards){};
	const bool mForwards;
};

// Advances the current presentation to the desired slide based on id
struct RequestPresentationSlideChange : public ds::RegisteredEvent<RequestPresentationSlideChange> {
	RequestPresentationSlideChange(const int slideId)
		: mSlideId(slideId) {}
	const int mSlideId;
};

// Resets the current step's layout
// If no screenId is passed, the screen is assumed by the position of the event
struct RequestPresentationStepRefresh : public ds::RegisteredEvent<RequestPresentationStepRefresh> {};

// If no screenId is passed, the screen is assumed by the position of the event
struct RequestPresentationEndEvent : public ds::RegisteredEvent<RequestPresentationEndEvent> {};

// Something changed with the current presentation. Check globals.mAllData for the current info
struct PresentationStatusUpdatedEvent : public ds::RegisteredEvent<PresentationStatusUpdatedEvent> {};

// A viewer was activated, changed order in the sprite list, was added, or removed
// Check globals.mViewerController's viewer list for the new list
// Note: this is not called when a viewer only changes size or position
struct ViewerUpdatedEvent : public ds::RegisteredEvent<ViewerUpdatedEvent> {};

struct ViewerAddedEvent : public ds::RegisteredEvent<ViewerAddedEvent> {
	ViewerAddedEvent(ds::model::ContentModelRef theNewViewerModel)
		: mModel(theNewViewerModel){};
	ds::model::ContentModelRef mModel;
};

/// Sent when a viewer starts animating off, assume it's dead after this
struct ViewerRemovedEvent : public ds::RegisteredEvent<ViewerRemovedEvent> {
	ViewerRemovedEvent(BaseElement* theViewer)
		: mViewer(theViewer){};
	BaseElement* mViewer;
};

struct RequestFullscreenViewer : public ds::RegisteredEvent<RequestFullscreenViewer> {
	RequestFullscreenViewer(BaseElement* viewer)
		: mViewer(viewer) {}
	BaseElement* mViewer;
};

struct RequestUnFullscreenViewer : public ds::RegisteredEvent<RequestUnFullscreenViewer> {
	RequestUnFullscreenViewer(BaseElement* viewer)
		: mViewer(viewer) {}
	BaseElement* mViewer;
};

/// A drawing is about to be saved, so hide ui and such that could potentially be blocking the drawing
struct RequestPreDrawingSave : public ds::RegisteredEvent<RequestPreDrawingSave> {};

struct RequestDrawingSave : public ds::RegisteredEvent<RequestDrawingSave> {
	RequestDrawingSave(ci::Surface theSurface, const int requestId, const std::string localPath)
		: mSurface(theSurface)
		, mRequestId(requestId)
		, mLocalPath(localPath){};
	ci::Surface		  mSurface;
	const int		  mRequestId;
	const std::string mLocalPath;
};

struct DrawingSaveComplete : public ds::RegisteredEvent<DrawingSaveComplete> {
	DrawingSaveComplete(const int requestId, const bool isErrored, const std::string errorString)
		: mRequestId(requestId)
		, mErrored(isErrored)
		, mErrorString(errorString) {}
	const int		  mRequestId;
	const bool		  mErrored;
	const std::string mErrorString;
};

struct RequestPinboardSave : public ds::RegisteredEvent<RequestPinboardSave>{
	RequestPinboardSave(ds::model::ContentModelRef model, const bool isAdd): mContentModel(model), mIsAdd(isAdd){}
	ds::model::ContentModelRef mContentModel;
	const bool mIsAdd;
};

struct PinboardSaveComplete : public ds::RegisteredEvent<PinboardSaveComplete>{
	PinboardSaveComplete(ds::model::ContentModelRef model, bool wasSuccess): mContentModel(model), mSaveSuccessful(wasSuccess){}
	ds::model::ContentModelRef mContentModel;
	bool mSaveSuccessful;
};

struct RequestNoteSave : public ds::RegisteredEvent<RequestNoteSave>{
	RequestNoteSave(const std::string& theNote, const bool alsoToPinboard, const int editingNode) : 
		mTheNote(theNote), mAlsoToPinboard(alsoToPinboard), mEditingNode(editingNode){}
	const std::string mTheNote;
	const bool mAlsoToPinboard; // once the note is saved, also add it to the pinboard
	const int mEditingNode; // if there's an existing node that's being edited. Send 0 or less to create a new node
};

struct NoteSaveComplete : public ds::RegisteredEvent<NoteSaveComplete>{
	NoteSaveComplete(const std::string& theNote, const int nodeId) : mTheNote(theNote), mNodeId(nodeId) {}
	const std::string mTheNote;
	const int mNodeId; 
};

struct RequestAppExit : public ds::RegisteredEvent<RequestAppExit> {};

struct PlatformChangeRequest : public ds::RegisteredEvent<PlatformChangeRequest> {
	PlatformChangeRequest(std::string path) { mPath = path; }
	std::string mPath = "";
};
struct PlatformChangedEvent : public ds::RegisteredEvent<PlatformChangedEvent> {};

struct ScheduleUpdated : public ds::RegisteredEvent<ScheduleUpdated> {
	ScheduleUpdated(ScheduleHandler* scheduleController, bool changed)
		: mScheduleHandler(scheduleController)
		, mScheduleChanged(changed) {}
	ScheduleHandler* mScheduleHandler;
	bool			 mScheduleChanged;
};

struct WafflesFilterEvent : public ds::RegisteredEvent<WafflesFilterEvent> {
	WafflesFilterEvent(std::string type, bool fromButton=false) : mType(type), mFromButton(fromButton) {}
	std::string mType;
	bool mFromButton;
};

struct WafflesTouchEvent : public ds::RegisteredEvent<WafflesTouchEvent> {
	WafflesTouchEvent(ci::vec3 point)
		: mPoint(point) {}
	ci::vec3 mPoint;
};

// ************************
// * Template events
// ************************
struct ChangeTemplateRequest : public ds::RegisteredEvent<ChangeTemplateRequest> {
	ChangeTemplateRequest(ds::model::ContentModelRef content)
		: mContent(content) {}
	ChangeTemplateRequest() {}

	ds::model::ContentModelRef mContent;
};
struct TemplateChangeStarted : public ds::RegisteredEvent<TemplateChangeStarted> {
	TemplateChangeStarted(ds::model::ContentModelRef content)
		: mContent(content) {}

	ds::model::ContentModelRef mContent;
};
struct TemplateChangeComplete : public ds::RegisteredEvent<TemplateChangeComplete> {};

// ************************
// * Engage events
// ************************
struct EngageStarted : public ds::RegisteredEvent<EngageStarted> {};
struct EngageEnded : public ds::RegisteredEvent<EngageEnded> {};

struct RequestEngagePresentation : public ds::RegisteredEvent<RequestEngagePresentation> {
	RequestEngagePresentation(ds::model::ContentModelRef content, bool hide_launcher = true)
		: mContent(content), mHideLauncher(hide_launcher){}
	RequestEngagePresentation() {}

	ds::model::ContentModelRef mContent;
	bool mHideLauncher;
};

struct RequestEngageNext : public ds::RegisteredEvent<RequestEngageNext> {};
struct RequestEngageBack : public ds::RegisteredEvent<RequestEngageBack> {};


// ************************
// * Background events
// ************************
struct TransitionRequest : public ds::RegisteredEvent<TransitionRequest> {};

struct TransitionReady : public ds::RegisteredEvent<TransitionReady> {};

struct TransitionComplete : public ds::RegisteredEvent<TransitionComplete> {};

struct ChangeBackgroundRequest : public ds::RegisteredEvent<ChangeBackgroundRequest> {
	ChangeBackgroundRequest(ds::model::ContentModelRef content)
		: mContent(content) {}
	ChangeBackgroundRequest() {}

	ds::model::ContentModelRef mContent;
};
struct BackgroundChangeComplete : public ds::RegisteredEvent<BackgroundChangeComplete> {};


struct NextAmbient : public ds::RegisteredEvent<NextAmbient> {};

// ************************
// * Template events
// ************************

// ************************
// * Engage events
// ************************

struct PresentationStateChanged : public ds::RegisteredEvent<PresentationStateChanged> {
	PresentationStateChanged(ds::model::ContentModelRef content)
		: mContent(content) {}
	PresentationStateChanged() {}

	ds::model::ContentModelRef mContent;
};

// ************************
// * Asset Events
// ************************
struct ViewerControllerStarted : public ds::RegisteredEvent<ViewerControllerStarted> {
	ViewerControllerStarted(waffles::ViewerController* theController)
		: mController(theController) {}

	waffles::ViewerController* mController;
};

struct ViewerInfo : public ds::RegisteredEvent<ViewerInfo> {
	ViewerInfo(int id, ci::Rectf bounds, ds::ui::Sprite* tex)
		: mId(id)
		, mBounds(bounds)
		, mTexture(tex) {}

	int				mId;
	ci::Rectf		mBounds;
	ds::ui::Sprite* mTexture;
};

struct RequestTemplatePrimaryNext : public ds::RegisteredEvent<RequestTemplatePrimaryNext> {};
struct RequestTemplatePrimaryPrev : public ds::RegisteredEvent<RequestTemplatePrimaryPrev> {};

struct RequestTemplateSecondaryNext : public ds::RegisteredEvent<RequestTemplateSecondaryNext> {};
struct RequestTemplateSecondaryPrev : public ds::RegisteredEvent<RequestTemplateSecondaryPrev> {};

struct RequestTemplateTextScroll : public ds::RegisteredEvent<RequestTemplateTextScroll> {
	RequestTemplateTextScroll(float scrollChange)
		: mScrollChange(scrollChange) {}

	float mScrollChange;
};
struct TemplateTextScrollChanged : public ds::RegisteredEvent<TemplateTextScrollChanged> {
	TemplateTextScrollChanged(bool canScroll, float scrollPercent)
		: mCanScroll(canScroll)
		, mScrollPercent(scrollPercent) {}

	bool  mCanScroll;
	float mScrollPercent;
};

struct TemplatePrimaryPageChange : public ds::RegisteredEvent<TemplatePrimaryPageChange> {
	TemplatePrimaryPageChange(int currentPage)
		: mCurrentPage(currentPage) {}

	int mCurrentPage;
};

struct TemplateSecondaryPageChange : public ds::RegisteredEvent<TemplateSecondaryPageChange> {
	TemplateSecondaryPageChange(int currentPage)
		: mCurrentPage(currentPage) {}

	int mCurrentPage;
};

/* struct RequestDotIndicatorNext : public ds::RegisteredEvent<RequestDotIndicatorNext> {};
struct RequestDotIndicatorBack : public ds::RegisteredEvent<RequestDotIndicatorBack> {}; */

/* struct RequestDotFeatureCard : public ds::RegisteredEvent<RequestDotFeatureCard> {};
struct RequestDotFeatureCardMedia : public ds::RegisteredEvent<RequestDotFeatureCardMedia> {}; */

// ************************
// * TUIO Object events
// ************************
struct TuioObjectEvent : public ds::RegisteredEvent<TuioObjectEvent> {
	TuioObjectEvent(const ds::ui::TouchInfo::Phase& phase, const ds::TuioObject& obj)
		: mObj(obj)
		, mPhase(phase){};
	const ds::TuioObject&			mObj;
	const ds::ui::TouchInfo::Phase& mPhase;
};

struct RequestTuioObjectTrigger : public ds::RegisteredEvent<RequestTuioObjectTrigger> {
	RequestTuioObjectTrigger(int id)
		: mId(id) {}
	int mId;
};

struct TuioObjectTriggered : public ds::RegisteredEvent<TuioObjectTriggered> {
	TuioObjectTriggered(int id)
		: mId(id) {}
	int mId;
};

struct RequestTuioObjectRelease : public ds::RegisteredEvent<RequestTuioObjectRelease> {
	RequestTuioObjectRelease(int id)
		: mId(id) {}
	int mId;
};

struct TuioObjectReleased : public ds::RegisteredEvent<TuioObjectReleased> {
	TuioObjectReleased(int id)
		: mId(id) {}
	int mId;
};

} // namespace waffles
