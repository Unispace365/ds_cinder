#include "stdafx.h"

#include "viewer_controller.h"

#include <cinder/Rand.h>
#include <cinder/app/App.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/media/player/pdf_player.h>
#include <ds/ui/panel/panel_layouts.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/file_meta_data.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
//#include "app/helpers.h"
//#include "events/app_events.h"
#include "waffles/waffles_events.h"
#include "waffles/model/viewer_creation_args.h"
#include "waffles/viewers/diagnostic_viewer/diagnostic_viewer.h"
#include "waffles/viewers/error_viewer/error_viewer.h"
#include "waffles/viewers/fullscreen_controller/fullscreen_controller.h"
#include "waffles/viewers/launcher/launcher.h"
#include "waffles/viewers/presentation_controller/presentation_controller.h"
#include "waffles/viewers/search/search_viewer.h"
#include "waffles/viewers/settings_viewer/settings_viewer.h"
#include "waffles/viewers/state_viewer/state_viewer.h"
#include "waffles/viewers/titled_media_viewer.h"

//using namespace downstream;

namespace {
static waffles::ViewerController* THIS_INSTANCE;
}

namespace waffles {

ViewerController::ViewerController(ds::ui::SpriteEngine& g, ci::vec2 size)
	: ds::ui::Sprite(g)
	, mEventClient(g) {

	THIS_INSTANCE = this;

	if (size.x < 0.f && size.y < 0.f) {
		mDisplaySize = ci::vec2(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	} else {
		mDisplaySize = size;
	}

	setSize(mDisplaySize.x, mDisplaySize.y);

	mBackgroundLayer = new ds::ui::Sprite(mEngine);
	mBackgroundLayer->setSize(mDisplaySize.x, mDisplaySize.y);
	addChildPtr(mBackgroundLayer);

	mNormalLayer = new ds::ui::Sprite(mEngine);
	mNormalLayer->setSize(mDisplaySize.x, mDisplaySize.y);
	addChildPtr(mNormalLayer);

	mTopLayer = new ds::ui::Sprite(mEngine);
	mTopLayer->setSize(mDisplaySize.x, mDisplaySize.y);
	addChildPtr(mTopLayer);

	mEventClient.listenToEvents<RequestViewerLaunchEvent>([this](const RequestViewerLaunchEvent& event) {
		handleRequestViewerLaunch(event);
		if (mRequestViewerLaunchCallback) mRequestViewerLaunchCallback(event);
	});

	mEventClient.listenToEvents<RequestCloseAllEvent>([this](const RequestCloseAllEvent& e) {
		const float deltaAnim = mEngine.getAnimDur() / (float)mViewers.size();
		float		delayey	  = deltaAnim * (float)mViewers.size();
		for (auto it : mViewers) {
			if ((!mEngine.isIdling() &&
				 (it->getViewerType() == VIEW_TYPE_PRESENTATION_CONTROLLER ||
				  it->getViewerType() == VIEW_TYPE_LAUNCHER || it->getViewerType() == VIEW_TYPE_SEARCH))) {

				it->sendToFront();
				continue;
			}

			if (!e.mCloseSlideContent && it->mCreationArgs.mAmSlideContent == true) continue;

			if (it->getViewerLayer() == ViewerCreationArgs::kViewLayerBackground) {
				animateViewerOff(it, delayey, ANIMATE_OFF_FADE);
			} else {
				animateViewerOff(it, delayey, ANIMATE_OFF_SHRINK);
			}
			delayey -= deltaAnim;
		}
	});

	mEventClient.listenToEvents<RequestArrangeEvent>([this](auto& e) { arrangeViewers(); });

	mEventClient.listenToEvents<RequestGatherEvent>([this](auto& e) { gatherViewers(e.mEventOrigin); });

	mEventClient.listenToEvents<RequestFullscreenViewer>([this](auto& e) { fullscreenViewer(e.mViewer, false); });

	mEventClient.listenToEvents<RequestUnFullscreenViewer>([this](auto& e) { unfullscreenViewer(e.mViewer, false); });

	mEventClient.listenToEvents<RequestGenericAdvance>([this](auto& e) {
		bool hadPdf = advancePDF(e.mForwards);
		if (!hadPdf) advancePresentation(e.mForwards);
	});

	mEventClient.listenToEvents<RequestPDFPageChange>([this](auto& e) { advancePDF(e.mForwards); });

	mEventClient.listenToEvents<RequestAppExit>([this](auto& e) {
		mEngine.getNotifier().notify(RequestCloseAllEvent());
		callAfterDelay([] { ci::app::App::get()->quit(); }, 2.0f);
	});
}

ViewerController* ViewerController::getInstance() {
	return THIS_INSTANCE;
}

std::vector<BaseElement*> ViewerController::getViewersOfType(const std::string& typeOfViewer) {
	std::vector<BaseElement*> viewers;
	for (auto it : mViewers) {
		if (it->getViewerType() == typeOfViewer) {
			viewers.push_back(it);
		}
	}

	return viewers;
}

std::vector<BaseElement*> ViewerController::getViewersWithResourceId(const ds::Resource::Id& resourceId) {
	std::vector<BaseElement*> returnElements;
	auto					  allViewers = getViewersOfType(VIEW_TYPE_TITLED_MEDIA_VIEWER);
	for (auto vit : allViewers) {
		if (vit && vit->getMedia().getPropertyResource("media").getDbId() == resourceId) {
			returnElements.push_back(vit);
		}
	}

	return returnElements;
}

void ViewerController::setLayerBounds(int viewLayer, ci::Rectf bounds) {
	if (viewLayer == ViewerCreationArgs::kViewLayerBackground && mBackgroundLayer) {
		mBackgroundLayer->setSize(bounds.getSize());
		mBackgroundLayer->setPosition(bounds.getUpperLeft());
	} else if (viewLayer == ViewerCreationArgs::kViewLayerNormal && mNormalLayer) {
		mNormalLayer->setSize(bounds.getSize());
		mNormalLayer->setPosition(bounds.getUpperLeft());
	} else if (viewLayer == ViewerCreationArgs::kViewLayerTop && mTopLayer) {
		mTopLayer->setSize(bounds.getSize());
		mTopLayer->setPosition(bounds.getUpperLeft());
	}
}

void ViewerController::addViewer(ViewerCreationArgs& args, const float delay) {
	const float screenWidth	 = mDisplaySize.x;
	const float screenHeight = mDisplaySize.y;
	if (!mNormalLayer) {
		DS_LOG_WARNING("Big problemo!");
		return;
	}

	auto loccy = globalToLocal(args.mLocation);
	if (args.mViewLayer == ViewerCreationArgs::kViewLayerTop) {
		loccy = mTopLayer->globalToLocal(args.mLocation);
	} else if (args.mViewLayer == ViewerCreationArgs::kViewLayerBackground) {
		loccy = mBackgroundLayer->globalToLocal(args.mLocation);
	} else {
		loccy = mNormalLayer->globalToLocal(args.mLocation);
	}
	if (loccy.x < 0.0f && loccy.y < 0.0f && loccy.z < 0.0f) {
		loccy.x = mDisplaySize.x / 2.0f;
		loccy.y = mDisplaySize.y / 2.0f;
		loccy.z = 0.0f;
	}

	// Special case where we just move around the viewer if there's only 1 of that type
	auto viewers = getViewersOfType(args.mViewType);
	if (!viewers.empty() && !viewers.front()->getIsAboutToBeRemoved() &&
		viewers.front()->getMaxNumberOfThisType() == 1) {

		auto sameType = viewers.front();

		if (args.mViewType == VIEW_TYPE_FULLSCREEN_CONTROLLER) {
			animateViewerOff(sameType, 0.0f, ANIMATE_OFF_SHRINK);
			return;
		}

		if (args.mFromCenter) {
			loccy = ci::vec3(loccy.x - sameType->getWidth() / 2.0f, loccy.y - sameType->getHeight() / 2.0f, 0.0f);
		}

		bool checkBoundsy = args.mCheckBounds;

		sameType->setMedia(args.mMediaRef);
		sameType->tweenStarted();
		sameType->tweenPosition(loccy, mEngine.getAnimDur(), 0.0f, ci::easeInOutQuad, [sameType, checkBoundsy] {
			sameType->tweenEnded();
			if (checkBoundsy) {
				sameType->checkBounds(false);
			}
		});
		sameType->activatePanel();

		return;
	}

	// In single-screen mode, only reset the position for non-media viewers after moving the current one
	if (args.mViewType != VIEW_TYPE_TITLED_MEDIA_VIEWER &&
		mEngine.getAppSettings().getString("screen:app:mode", 0, "wall") == "single") {
		float yPercent = mEngine.getAppSettings().getFloat("single_app:y_panel_percent", 0, 2.0f / 3.0f);
		loccy		   = ci::vec3(screenWidth / 2.0f, screenHeight * yPercent, 0.0f);
		if (args.mViewType == VIEW_TYPE_FULLSCREEN_CONTROLLER) {
			loccy.y += 250.0f; // a little hack so fsc's show up below pres controllers
		}
	}

	BaseElement* newViewer = nullptr;

	if (args.mViewType == VIEW_TYPE_TITLED_MEDIA_VIEWER) {
		auto theResource = args.mMediaRef.getPropertyResource("media");
		if (args.mMediaRef.getPropertyString("type") != MEDIA_TYPE_CAPTURE &&
			theResource.getType() != ds::Resource::WEB_TYPE && theResource.getType() != ds::Resource::YOUTUBE_TYPE &&
			theResource.getType() != ds::Resource::VIDEO_STREAM_TYPE) {

			if (ds::safeFileExistsCheck(theResource.getAbsoluteFilePath())) {
				newViewer = new TitledMediaViewer(mEngine);
			} else {

				ds::model::ContentModelRef errorModel;
				std::string errorMessage = "We couldn't load this piece of media because the file couldn't be found.";

				errorModel.setProperty("name", std::string("Sorry!"));
				errorModel.setProperty("error", errorMessage);
				errorModel.setPropertyResource("media", theResource);
				errorModel.setProperty("media_path", theResource.getAbsoluteFilePath());
				errorModel.setProperty("media_name", args.mMediaRef.getPropertyString("name"));
				mEngine.getNotifier().notify(RequestViewerLaunchEvent(
					ViewerCreationArgs(errorModel, VIEW_TYPE_ERROR, args.mLocation, ViewerCreationArgs::kViewLayerTop,
									   0, args.mFromCenter)));

				return;
			}
		} else {
			newViewer = new TitledMediaViewer(mEngine);
		}

	} else if (args.mViewType == VIEW_TYPE_LAUNCHER) {
		newViewer = new Launcher(mEngine);
		// args.mViewLayer = ViewerCreationArgs::kViewLayerTop;
	} else if (args.mViewType == VIEW_TYPE_LAUNCHER_PERSISTANT) {
		newViewer = new Launcher(mEngine, true);
		// args.mViewLayer = ViewerCreationArgs::kViewLayerTop;
	} else if (args.mViewType == VIEW_TYPE_SEARCH) {
		newViewer = new SearchViewer(mEngine, VIEW_TYPE_SEARCH);
	} else if (args.mViewType == VIEW_TYPE_SELECT_MEDIA_AMBIENT ||
			   args.mViewType == VIEW_TYPE_SELECT_MEDIA_BACKGROUND) {
		newViewer = new SearchViewer(mEngine, args.mViewType);
	} else if (args.mViewType == VIEW_TYPE_PRESENTATION_CONTROLLER) {
		newViewer = new PresentationController(mEngine);
	} else if (args.mViewType == VIEW_TYPE_SETTINGS) {
		newViewer = new SettingsViewer(mEngine);
	} else if (args.mViewType == VIEW_TYPE_FULLSCREEN_CONTROLLER) {
		newViewer = new FullscreenController(mEngine);
	} else if (args.mViewType == VIEW_TYPE_DIAGNOSTIC) {
		newViewer = new DiagnosticViewer(mEngine);
	} else if (args.mViewType == VIEW_TYPE_STATE_VIEWER) {
		newViewer = new StateViewer(mEngine);
	} else if (args.mViewType == VIEW_TYPE_ERROR) {
		newViewer = new ErrorViewer(mEngine);
	}

	if (!newViewer) {
		DS_LOG_WARNING("View type not recognized! " << args.mViewType);
		return;
	}


	if (args.mViewLayer == ViewerCreationArgs::kViewLayerBackground && mBackgroundLayer) {
		mBackgroundLayer->addChildPtr(newViewer);
		newViewer->sendToBack();
		newViewer->setBoundingArea(ci::Rectf(ci::vec2(0.f), mBackgroundLayer->getSize()));
	} else if (args.mViewLayer == ViewerCreationArgs::kViewLayerNormal && mNormalLayer) {
		mNormalLayer->addChildPtr(newViewer);
		if (args.mViewType == VIEW_TYPE_LAUNCHER || args.mViewType == VIEW_TYPE_LAUNCHER_PERSISTANT) {
			auto widthDifference = (mTopLayer->getWidth() - mNormalLayer->getWidth()) / 2.f;
			newViewer->setBoundingArea(ci::Rectf(ci::vec2(-widthDifference, 0.f),
												 ci::vec2(-widthDifference, 0.f) + ci::vec2(mTopLayer->getSize())));
		} else {
			newViewer->setBoundingArea(ci::Rectf(ci::vec2(0.f), mNormalLayer->getSize()));
		}
		// newViewer->setBoundingArea(ci::Rectf(ci::vec2(0.f), mNormalLayer->getSize()));
	} else if (args.mViewLayer == ViewerCreationArgs::kViewLayerTop && mTopLayer) {
		mTopLayer->addChildPtr(newViewer);
		newViewer->setBoundingArea(ci::Rectf(ci::vec2(0.f), mTopLayer->getSize()));
	} else {
		DS_LOG_WARNING("Invalid view layer specified for new viewer " << args.mMediaRef.getPropertyString("name"));
		newViewer->release();
		return;
	}

	DS_LOG_INFO("Launching viewer of type " << newViewer->getViewerType() << " "
											<< args.mMediaRef.getPropertyResource("media").getAbsoluteFilePath());

	mViewers.push_back(newViewer);

	newViewer->setMedia(args.mMediaRef);
	newViewer->setCreationArgs(args);
	newViewer->setViewerLayer(args.mViewLayer);

	const float viewerScale = mEngine.getWafflesSettings().getFloat("viewer:master_scale", 0, 1.0f);

	if (newViewer->canResize()) {
		if (args.mEnforceMinSize &&
			args.mStartWidth < mEngine.getWafflesSettings().getFloat("media_viewer:min_size", 0, 100.0f)) {
			newViewer->setViewerWidth(mEngine.getWafflesSettings().getFloat("media_viewer:default_size", 0, 400.0f) /
									  viewerScale);
		} else {
			newViewer->setAbsoluteSizeLimits(ci::vec2(args.mStartWidth / viewerScale, 200.f),
											 ci::vec2(99999.f, 99999.f));
			// newViewer->setViewerWidth(args.mStartWidth / viewerScale);
			newViewer->setSizeLimits();
			newViewer->setViewerWidth(args.mStartWidth / viewerScale);
		}
	}

	if (args.mFromCenter) {
		newViewer->setPosition(loccy.x - newViewer->getWidth() * viewerScale / 2.0f,
							   loccy.y - newViewer->getHeight() * viewerScale / 2.0f);
	} else {
		newViewer->setPosition(loccy);
	}

	newViewer->setScale(viewerScale);

	if (args.mCheckBounds) {
		newViewer->checkBounds(true);
	}

	if (newViewer->canFullScreen() && args.mFullscreen) {
		fullscreenViewer(newViewer, true, args.mShowFullscreenController);
		const bool webEnough = args.mMediaRef.getPropertyResource("media").getType() == ds::Resource::WEB_TYPE ||
							   args.mMediaRef.getPropertyResource("media").getType() == ds::Resource::YOUTUBE_TYPE;
		if (webEnough && args.mTouchEvents) {
			if (auto tmv = dynamic_cast<waffles::TitledMediaViewer*>(newViewer)) {
				tmv->setInterfaceLocked(true);
			}
		}
	}

	newViewer->animateOn(delay);

	newViewer->setCloseRequestCallback([this, newViewer]() { animateViewerOff(newViewer, 0.0f, ANIMATE_OFF_SHRINK); });
	newViewer->setActivatedCallback([this, newViewer] { viewerActivated(newViewer); });

	enforceViewerLimits(newViewer);

	mEngine.getNotifier().notify(ViewerUpdatedEvent());
	mEngine.getNotifier().notify(ViewerAddedEvent(args.mMediaRef));
}

void ViewerController::enforceViewerLimits(BaseElement* viewer) {
	if (!viewer) return;

	auto typedViewers = getViewersOfType(viewer->getViewerType());

	std::vector<BaseElement*> onThisScreen;
	for (auto it : typedViewers) {
		onThisScreen.push_back(it);
	}

	if (onThisScreen.empty() || onThisScreen.size() < viewer->getMaxNumberOfThisType()) return;

	size_t overflowAmount = onThisScreen.size() - viewer->getMaxNumberOfThisType();
	for (size_t i = 0; i < overflowAmount; i++) {
		// just in case my math sucks
		if (i > onThisScreen.size() - 1) break;

		animateViewerOff(onThisScreen[i], 0.0f, ANIMATE_OFF_SHRINK);
	}
}

void ViewerController::viewerActivated(BaseElement* be) {
	if (be) {
		auto found = std::find(mViewers.begin(), mViewers.end(), be);
		if (found != mViewers.end() && be != mViewers.back()) {
			mViewers.erase(found);
			mViewers.push_back(be);
			mEngine.getNotifier().notify(ViewerUpdatedEvent());
		}
	}
}

void ViewerController::handleRequestViewerLaunch(const RequestViewerLaunchEvent& event) {
	auto e = event;
	if (e.mUserStringData.empty()) {
		if (e.mViewerArgs.mMediaRef.getPropertyString("type") == MEDIA_TYPE_PRESENTATION) {
			startPresentation(e.mViewerArgs.mMediaRef, e.mViewerArgs.mLocation,
							  e.mViewerArgs.mShowPresentationController);
		} else if (e.mViewerArgs.mMediaRef.getPropertyString("type_key") == "slide-grid" ||
				   e.mViewerArgs.mMediaRef.getPropertyString("type_key") == "slide" ||
				   e.mViewerArgs.mMediaRef.getPropertyString("type_key") == "custom_layout_template") {
			loadPresentationSlide(e.mViewerArgs.mMediaRef);
		} else if (e.mViewerArgs.mMediaRef.getPropertyString("type_key") == MEDIA_TYPE_PINBOARD) {
			auto thePinboard = e.mViewerArgs.mMediaRef;

			ViewerCreationArgs args;
			args.mLocation = e.mViewerArgs.mLocation + ci::vec3(200.f, 100.f, 0);
			args.mViewType = VIEW_TYPE_TITLED_MEDIA_VIEWER;
			for (auto&& [key, pinboardItem] : thePinboard.getReferences("pinboard_items")) {
				args.mMediaRef = pinboardItem;
				addViewer(args);
				args.mLocation += ci::vec3(200.f, 100.f, 0);
			}
			// arrangeViewers();
		} else {
			/* auto parent = getRecordByUid(mEngine, e.mViewerArgs.mMediaRef.getPropertyString("parent_uid"));
			if (parent.getPropertyString("type_key") == "ambient_playlist") {

				mEngine.getNotifier().notify(RequestEngagePresentation(e.mViewerArgs.mMediaRef));
			} else { */
			addViewer(e.mViewerArgs);
			// }
		}
	} else {
		ViewerCreationArgs args;
		args.mLocation = e.mEventOrigin;
		auto tokens	   = ds::split(e.mUserStringData, " ! ", true);
		for (auto it : tokens) {
			auto colony = it.find(":");
			if (colony != std::string::npos) {
				std::string paramType  = it.substr(0, colony);
				std::string paramValue = it.substr(colony + 1);

				if (paramType.empty() || paramValue.empty()) continue;
				if (paramType == "fullscreen") {
					args.mFullscreen = ds::parseBoolean(paramValue);
				} else if (paramType == "view_type") {
					args.mViewType = paramValue;
				} else if (paramType == "location") {
					args.mLocation = ds::parseVector(paramValue);
				} else if (paramType == "start_width") {
					args.mStartWidth = ds::string_to_float(paramValue);
				} else if (paramType == "from_center") {
					args.mFromCenter = ds::parseBoolean(paramValue);
				} else if (paramType == "view_layer") {
					args.mViewLayer = ds::string_to_int(paramValue);
				} else if (paramType == "media_path") {
					if (args.mViewType == VIEW_TYPE_TITLED_MEDIA_VIEWER) {
						args.mMediaRef.setPropertyResource("media", ds::Resource(ds::Environment::expand(paramValue)));
					} else {
						args.mMediaRef.setProperty("media_path", paramValue);
					}
				} else if (paramType == "drawing") {
					if (paramValue == "on") {
						args.mStartDrawing = true;
					}
				}
			}
		}

		addViewer(args);
	}
}

void ViewerController::animateViewerOff(BaseElement* viewer, const float delayey, const int style) {
	if (!viewer || viewer->getIsAboutToBeRemoved()) return;

	removeFullscreenDarkener(viewer);

	/* if(viewer->mIsFullscreen){
		unfullscreenViewer(viewer, true);
	} */
	viewer->setAboutToBeRemoved(true);
	viewer->tweenStarted();

	auto completeCallback = [this, viewer]() {
		removeViewer(viewer);
	};

	viewer->tweenAnimateOff(true, 0.f, 0.0f, completeCallback);
	/* if (style == 1) {
		viewer->tweenOpacity(0.0f, mEngine.getAnimDur(), delayey, ci::easeNone, completeCallback);
	} else if (style == 2) {
		viewer->tweenPosition(ci::vec3(viewer->getPosition().x, mDisplaySize.y, viewer->getPosition().z),
							  mEngine.getAnimDur(), delayey, ci::easeInCubic, completeCallback);
	} else {
		viewer->tweenPosition(ci::vec3(viewer->getPosition().x + viewer->getScaleWidth() / 4.0f,
									   viewer->getPosition().y + 100.0f + viewer->getScaleHeight() / 4.0f,
									   viewer->getPosition().z),
							  mEngine.getAnimDur(), delayey, ci::easeInCubic);
		viewer->tweenScale(viewer->getScale() / 2.0f, mEngine.getAnimDur(), delayey, ci::easeInCubic, completeCallback);
	} */

	mEngine.getNotifier().notify(ViewerUpdatedEvent());
	mEngine.getNotifier().notify(ViewerRemovedEvent(viewer));
}

void ViewerController::removeViewer(BaseElement* viewer) {
	removeFullscreenDarkener(viewer);

	auto found = std::find(mViewers.begin(), mViewers.end(), viewer);
	if (found != mViewers.end()) {
		mViewers.erase(found);
	}

	if (viewer) {
		viewer->release();
	}

	mEngine.getNotifier().notify(ViewerUpdatedEvent());
}

void ViewerController::arrangeViewers() {

	std::vector<BaseElement*> gridViewers;
	for (auto be : mViewers) {
		if (be->getViewerType() == VIEW_TYPE_LAUNCHER || be->getViewerType() == VIEW_TYPE_SEARCH) continue;
		if (be->mCreationArgs.mAmSlideContent == true) continue;

		removeFullscreenDarkener(be);

		if (be->getViewerLayer() != ViewerCreationArgs::kViewLayerNormal) {
			continue;
		}

		if (be && be->getIsAboutToBeRemoved()) {
			continue;
		}

		if (be && be->canArrange() && !be->getIsFatalErrored()) {

			be->hideTitle();

			gridViewers.push_back(be);

		} else {
			animateViewerOff(be, 0.0f, ANIMATE_OFF_FADE);
		}
	}

	if (gridViewers.size() < 1) return;

	if (gridViewers.size() == 1 && gridViewers.front()->canFullScreen()) {
		fullscreenViewer(gridViewers.front(), false);
	} else {
		std::vector<ds::ui::BasePanel*> basePanels;
		for (auto it : gridViewers) {
			basePanels.push_back(it);
		}
		auto boundy = ci::Rectf(ci::vec2(0.f), mNormalLayer->getSize());

		ds::ui::PanelLayouts::binPack(basePanels, boundy, 5.0f, mEngine.getAnimDur());
	}
}

void ViewerController::gatherViewers(const ci::vec3& location) {
	auto localLoc = location;

	if (mNormalLayer) {
		localLoc = mNormalLayer->globalToLocal(location);
	}

	for (auto it : mViewers) {
		if (it->getViewerLayer() == ViewerCreationArgs::kViewLayerTop) continue;
		if (it->getViewerType() == VIEW_TYPE_LAUNCHER || it->getViewerType() == VIEW_TYPE_SEARCH) continue;
		if (it->mCreationArgs.mAmSlideContent == true) continue;

		gatherAviewer(it, localLoc);
	}
}

void ViewerController::gatherAviewer(BaseElement* viewer, const ci::vec3& location) {
	if (!viewer) return;

	removeFullscreenDarkener(viewer);

	float destWidth = viewer->getWidth();
	if (viewer->canResize()) {
		destWidth = viewer->getMinSize().x;
		viewer->animateWidthTo(viewer->getMinSize().x);
	}

	float destHeight = destWidth / (viewer->getWidth() / viewer->getHeight());
	viewer->tweenStarted();
	viewer->tweenPosition(ci::vec3(location.x + ci::randFloat(-300.0f, 300.0f) - destWidth / 2.0f,
								   location.y + ci::randFloat(-300.0f, 300.0f) - destHeight / 2.0f, 0.0f),
						  mEngine.getAnimDur(), 0.0f, ci::easeInOutQuad, [viewer] { viewer->tweenEnded(); });
}

bool ViewerController::advancePDF(const bool forwards) {
	if (mViewers.empty()) return false;

	TitledMediaViewer* titledViewer = nullptr;
	for (auto it = mViewers.rbegin(); it < mViewers.rend(); ++it) {
		titledViewer = dynamic_cast<TitledMediaViewer*>((*it));
		if (titledViewer) break;
	}

	if (titledViewer) {
		auto mediaPlayer = titledViewer->getMediaPlayer();

		if (mediaPlayer) {
			auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(mediaPlayer->getPlayer());
			if (pdfPlayer) {
				if (forwards) {
					pdfPlayer->nextPage();
				} else {
					pdfPlayer->prevPage();
				}

				return true;
			}
		}
	}

	return false;
}

void ViewerController::startPresentation(ds::model::ContentModelRef newPresentation, const ci::vec3& startLocation,
										 const bool showController) {

	auto thePres = mEngine.mContent.getChildByName("current_presentation");
	thePres.setProperty("presentation_id", newPresentation.getId());
	if (!newPresentation.getChildren().empty()) {
		thePres.setProperty("slide_id", newPresentation.getChildren().front().getId());
	} else {
		thePres.setProperty("slide_id", 0);
	}

	setPresentationSlide(thePres.getPropertyInt("slide_id"));

	if (showController) {
		auto vca =
			ViewerCreationArgs(ds::model::ContentModelRef(), VIEW_TYPE_PRESENTATION_CONTROLLER, startLocation, 2);
		mEngine.getNotifier().notify(RequestViewerLaunchEvent(vca));
	}
}

void ViewerController::endPresentation() {
	auto thePres = mEngine.mContent.getChildByName("current_presentation");
	thePres.setProperty("presentation_id", 0);
	thePres.setProperty("slide_id", 0);
}

void ViewerController::advancePresentation(const bool forwards) {

	auto thePres	= mEngine.mContent.getChildByName("current_presentation");
	auto actualPres = mEngine.mContent.getChildByName("cms_root")
						  .getDescendant("waffles_nodes", thePres.getPropertyInt("presentation_id"));
	if (actualPres.getChildren().empty()) {
		DS_LOG_WARNING("Tried to advance a presentation slide, but it doesn't have any slides");
		mEngine.getNotifier().notify(PresentationStatusUpdatedEvent());
		return;
	}

	auto curSlideId = thePres.getPropertyInt("slide_id");
	int	 newSlideId = 0;
	if (forwards) {
		bool foundSlide = false;
		for (auto it : actualPres.getChildren()) {
			if (foundSlide) {
				newSlideId = it.getId();
				break;
			}
			if (it.getId() == curSlideId) {
				foundSlide = true;
			}
		}

		// wrap around
		if (newSlideId == 0) newSlideId = actualPres.getChild(0).getId();

	} else {

		for (auto it : actualPres.getChildren()) {
			if (it.getId() == curSlideId) break;
			newSlideId = it.getId();
		}

		// wrap around
		if (newSlideId == 0) newSlideId = actualPres.getChildren().back().getId();
	}

	setPresentationSlide(newSlideId);
}

void ViewerController::setPresentationSlide(int slideId) {

	auto thePres	 = mEngine.mContent.getChildByName("current_presentation");
	auto actualSlide = mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", slideId);
	if (actualSlide.empty()) {
		DS_LOG_WARNING("Tried to set a presentation slide, but it's empty!");
		mEngine.getNotifier().notify(PresentationStatusUpdatedEvent());
		return;
	}
	auto actualPres = mEngine.mContent.getChildByName("cms_root")
						  .getDescendant("waffles_nodes", actualSlide.getPropertyInt("parent_id"));
	if (actualPres.getChildren().empty()) {
		DS_LOG_WARNING("Tried to set a presentation slide, but it doesn't have any slides");
		mEngine.getNotifier().notify(PresentationStatusUpdatedEvent());
		return;
	}

	ds::model::ContentModelRef theSlideRef;
	for (auto it : actualPres.getChildren()) {
		if (it.getId() == slideId) {
			theSlideRef = it;
			break;
		}
	}

	if (theSlideRef.empty()) {
		DS_LOG_WARNING("Slide id " << slideId << " not found in the current presentation");
		return;
	}

	loadPresentationSlide(theSlideRef);
}

void ViewerController::loadPresentationSlide(ds::model::ContentModelRef slideRef) {
	auto helper = waffles::WafflesHelperFactory::getDefault();
	auto thePres	 = helper->getRecordByUid(slideRef.getPropertyString("parent_uid"));
	auto actualSlide = slideRef;

	auto theType = slideRef.getPropertyString("type_key");

	// if the slide is hotspots (deprecated) or media, just load it
	if (theType == "slide-hotspots" || theType == MEDIA_TYPE_FILE_CMS) {
		std::vector<BaseElement*> leftoverViewers;
		for (auto it : mViewers) {
			if (it->getViewerLayer() != ViewerCreationArgs::kViewLayerNormal) continue;
			leftoverViewers.emplace_back(it);
		}

		for (auto it : leftoverViewers) {
			animateViewerOff(it, 0.0f, ANIMATE_OFF_FADE);
		}

		loadSlideBackground(slideRef);

		ViewerCreationArgs bacckyArgs = ViewerCreationArgs(
			slideRef, VIEW_TYPE_TITLED_MEDIA_VIEWER, ci::vec3(mDisplaySize.x / 2.0f, mDisplaySize.y / 2.0f, 0.0f));

		bacckyArgs.mViewLayer				 = ViewerCreationArgs::kViewLayerNormal;
		bacckyArgs.mStartWidth				 = 0.0f;
		bacckyArgs.mFromCenter				 = true;
		bacckyArgs.mFullscreen				 = true;
		bacckyArgs.mShowFullscreenController = false;
		bacckyArgs.mCheckBounds				 = false;

		addViewer(bacckyArgs);

	} else if (theType == "slide-grid" || theType == "custom_layout_template" || theType == "slide") {
		loadSlideComposite(slideRef);
	}

	mEngine.getNotifier().notify(PresentationStatusUpdatedEvent());
}

void ViewerController::loadSlideComposite(ds::model::ContentModelRef slideRef) {
	auto helper = waffles::WafflesHelperFactory::getDefault();
	std::vector<BaseElement*> leftoverViewers;
	for (auto it : mViewers) {
		if (it->getViewerLayer() != ViewerCreationArgs::kViewLayerNormal ||
			(it->getViewerType() == VIEW_TYPE_PRESENTATION_CONTROLLER || it->getViewerType() == VIEW_TYPE_LAUNCHER))
			continue;
		leftoverViewers.emplace_back(it);
	}
	loadSlideBackground(slideRef);

	std::string layoutStyle = slideRef.getPropertyString("layout_option_save_value");
	if (slideRef.getPropertyString("type") == "slide-grid") {
		layoutStyle = "auto";
	}

	if (layoutStyle.empty() || layoutStyle == "none") layoutStyle = "letterbox";

	float delay			   = mEngine.getAnimDur() / 2.0f;
	delay				   = delay + (delay * float(slideRef.getChildren().size()));
	float		deltaDelay = mEngine.getAnimDur() / 2.0f;
	const float ww		   = mNormalLayer->getWidth();
	const float wh		   = mNormalLayer->getHeight();
	const float wasp	   = ww / wh;

	const ci::vec2 compositeRatio = mEngine.getWafflesSettings().getVec2("composite:aspect_ratio", 0, ci::vec2(16.f, 9.f));
	const float	   slideAspect	  = compositeRatio.x / compositeRatio.y;

	// effective sizes
	float ew = ww;
	float eh = wh;

	float fillXScale = 1.0f;
	float fillYScale = 1.0f;

	float xp = 0.0f;
	float yp = 0.0f;


	if (wasp > slideAspect) {
		ew		   = wh * (compositeRatio.x / compositeRatio.y);
		fillXScale = ww / ew;
	} else {
		eh		   = ww * (compositeRatio.y / compositeRatio.x);
		fillYScale = wh / eh;
	}
	xp = ww / 2.0f - ew / 2.0f;
	yp = wh / 2.0f - eh / 2.0f;


	auto sortedMedia = slideRef.getChildren();
	std::reverse(sortedMedia.begin(), sortedMedia.end());

	for (auto newMedia : sortedMedia) {
		// filter out the specific background nodes
		// if (newMedia.getName() != "waffles_nodes") continue;

		// if this is blank and it's NOT a video stream, ditch it.
		if (newMedia.getPropertyResource("media").getDbId().empty() &&
			newMedia.getPropertyResource("media").getAbsoluteFilePath().empty() &&
			newMedia.getPropertyResource("media").getType() != ds::Resource::VIDEO_STREAM_TYPE)
			continue;

		auto theResource = newMedia.getPropertyResource("media");

		ci::vec3 thePos	  = ci::vec3(ww / 2.0f, wh / 2.0f, 0.0f);
		float	 theWidth = -100.0f;

		auto compositeKey = helper->getCompositeKeyForPlatform();

		auto posX = (newMedia.getPropertyFloat(compositeKey+"_x") > 0.f) ? newMedia.getPropertyFloat(compositeKey+"_x")
																		: newMedia.getPropertyFloat(compositeKey+"_x");
		auto posY = (newMedia.getPropertyFloat(compositeKey+"_y") > 0.f) ? newMedia.getPropertyFloat(compositeKey+"_y")
																		: newMedia.getPropertyFloat(compositeKey+"_y");
		auto theW = (newMedia.getPropertyFloat(compositeKey+"_w") > 0.f) ? newMedia.getPropertyFloat(compositeKey+"_w")
																		: newMedia.getPropertyFloat(compositeKey+"_w");

		if (theW <= 0.f) {
			theW = 0.25f;
		}

		if (layoutStyle == "letterbox") {
			thePos.x = posX * ew + xp;
			thePos.y = posY * eh + yp;
			theWidth = theW * ew;
		} else if (layoutStyle == "fill") {
			// the source media size
			float mediaW = theResource.getWidth();
			float mediaH = theResource.getHeight();
			float mediaA = mediaW / mediaH;

			// find the stretched destination area of the media
			// basically stretching the original media target area by the amount that the world size is stretched away
			// from 16:9
			float destinationW = theW * ew * fillXScale;
			float destinationH = (theW * ew / mediaA) * fillYScale;
			float destinationA = destinationW / destinationH;

			// find the relative center point of the arranged asset in the stretched world space
			ci::vec2 theCenter;
			theCenter.x = posX * ww + destinationW / 2.0f;
			theCenter.y = posY * wh + destinationH / 2.0f;

			// letterbox the media into that stretched destination area
			if (mediaA > destinationA) {
				theWidth = destinationW;
			} else {
				theWidth = destinationH * mediaA;
			}

			// center the media around the relative center
			thePos.x = theCenter.x - theWidth / 2.0f;
			thePos.y = theCenter.y - (theWidth / mediaA) / 2.0f;
		} else if (layoutStyle == "expand") {

			// the source media size
			float mediaW = theResource.getWidth();
			float mediaH = theResource.getHeight();
			float mediaA = mediaW / mediaH;

			// find the stretched destination area of the media
			// basically stretching the original media target area by the amount that the world size is stretched away
			// from 16:9
			float destinationW = theW * ew * fillXScale;
			float destinationH = (theW * ew / mediaA) * fillYScale;

			// find the relative center point of the arranged asset in the stretched world space
			ci::vec2 theCenter;
			theCenter.x = posX * ww + destinationW / 2.0f;
			theCenter.y = posY * wh + destinationH / 2.0f;

			theWidth = theW * ww;
			/*
			// letterbox the media into that stretched destination area
			if(mediaA < destinationA) {
				theWidth = destinationW;
			} else {
				theWidth = destinationH * mediaA;
			}
			*/

			// center the media around the relative center
			thePos.x = theCenter.x - theWidth / 2.0f;
			thePos.y = theCenter.y - (theWidth / mediaA) / 2.0f;
		}

		ViewerCreationArgs args =
			ViewerCreationArgs(newMedia, VIEW_TYPE_TITLED_MEDIA_VIEWER, mNormalLayer->localToGlobal(thePos),
							   ViewerCreationArgs::kViewLayerNormal, theWidth, false, false, false);

		args.mShowFullscreenController = false;
		args.mEnforceMinSize		   = false; // danger zone? who cares!

		args.mFullscreen = false;
		// newMedia.getPropertyBool("media_float_fullscreen");
		args.mTouchEvents = newMedia.getPropertyBool("touch_events");
		// newMedia.getPropertyBool("media_float_touch");
		args.mStartLocked = args.mTouchEvents;
		args.mAutoStart	  = newMedia.getPropertyBool("autoplay");
		// newMedia.getPropertyBool("media_float_autoplay");
		args.mLooped = newMedia.getPropertyBool("loop");
		// newMedia.getPropertyBool("media_float_loop");
		args.mVolume = newMedia.getPropertyFloat("volume");
		// newMedia.getPropertyInt("media_float_volume");
		args.mPage		 = 0;
		args.mStartWidth = theWidth;
		// newMedia.getPropertyInt("media_float_page");
		args.mUseHotspots	 = true;
		args.mAmSlideContent = true;


		bool found = false;
		for (auto mit = leftoverViewers.begin(); mit < leftoverViewers.end(); ++mit) {
			auto leViewer = (*mit);
			if (leViewer->getIsAboutToBeRemoved()) continue;
			if (leViewer->getIsFatalErrored()) continue;
			if (leViewer->getMediaRotation() != 0) continue; // discard rotated viewers
			if (leViewer->getMedia().getPropertyResource("media") == newMedia.getPropertyResource("media")) {
				leViewer->sendToFront();
				leViewer->hideTitle();
				leViewer->setCreationArgs(args);

				if (leViewer->getIsFullscreen() != args.mFullscreen) {
					if (args.mFullscreen) {
						fullscreenViewer(leViewer, true, false);
					} else {
						unfullscreenViewer(leViewer, true);
					}
				}

				if (!args.mFullscreen) {
					// overrides the animation from unfullscreenViewer if applicable and that's what we want
					float viewerScale = leViewer->getScale().x;
					if (viewerScale == 0.0f) viewerScale = 0.0001f;
					leViewer->animateWidthTo(args.mStartWidth / viewerScale);
					leViewer->tweenPosition(args.mLocation, mEngine.getAnimDur(), 0.0f, ci::easeInOutQuad);
				}

				found = true;
				leftoverViewers.erase(mit);
				break;
			}
		}

		if (!found) {
			addViewer(args, delay);
		}

		delay -= deltaDelay;
	}


	for (auto it : leftoverViewers) {
		animateViewerOff(it, mEngine.getAnimDur(), ANIMATE_OFF_FADE);
	}

	if (layoutStyle == "auto") {
		arrangeViewers();
	}
}

void ViewerController::loadSlideBackground(ds::model::ContentModelRef slideRef) {

	std::string appKey = mEngine.getAppSettings().getString("media_backgrounds:app_key");

	/* auto cmsPlatform = mEngine.mContent.getChildByName("cms_platforms").getChild(0);
	if (!cmsPlatform.empty()) {
		auto aspectRation = cmsPlatform.getPropertyString("aspect_ratio");
		if (aspectRation == "16:9") {
			appKey = "background_media";
		} else if (!aspectRation.empty()) {
			appKey = "background_wide_media_res";
		}
	} */

	ds::Resource backgroundMediaRes;

	backgroundMediaRes = slideRef.getPropertyResource(appKey);
	if (backgroundMediaRes.empty()) {
		backgroundMediaRes = slideRef.getPropertyResource("background_media");
	}


	// if there was no media backgrounds, fall back to this slide
	if (backgroundMediaRes.empty()) {
		backgroundMediaRes = slideRef.getPropertyResource("media");
	}

	// if this slide doesn't have a background, check to see if it's in a presentation that does
	if (backgroundMediaRes.empty()) {
		auto actualPres = mEngine.mContent.getChildByName("cms_root")
							  .getDescendant("waffles_nodes", slideRef.getPropertyInt("parent_id"));
		backgroundMediaRes = actualPres.getPropertyResource("background_media");
	}

	bool alreadyExists = false;

	for (auto it : mViewers) {
		if (it->getViewerLayer() == ViewerCreationArgs::kViewLayerBackground) {
			auto viewerMedia = it->getMedia().getPropertyResource("media");

			if (!it->getIsAboutToBeRemoved() &&
				viewerMedia.getAbsoluteFilePath() == backgroundMediaRes.getAbsoluteFilePath() &&
				viewerMedia.getCrop().x1 - backgroundMediaRes.getCrop().x1 <= std::numeric_limits<float>::epsilon() &&
				viewerMedia.getCrop().y1 - backgroundMediaRes.getCrop().y1 <= std::numeric_limits<float>::epsilon() &&
				viewerMedia.getCrop().x2 - backgroundMediaRes.getCrop().x2 <= std::numeric_limits<float>::epsilon() &&
				viewerMedia.getCrop().y2 - backgroundMediaRes.getCrop().y2 <= std::numeric_limits<float>::epsilon()) {

				alreadyExists = true;
			} else {
				animateViewerOff(it, mEngine.getAnimDur(), ANIMATE_OFF_FADE);
			}
		}
	}

	const float scW = mDisplaySize.x;
	const float scH = mDisplaySize.y;

	// create a background media item background or leave it alone if it exists
	if (!alreadyExists && !backgroundMediaRes.empty()) {

		ds::model::ContentModelRef backgroundMedia;
		backgroundMedia.setPropertyResource("media", backgroundMediaRes);
		backgroundMedia.setProperty("type", MEDIA_TYPE_FILE_CMS);

		float outWid = scW;
		float engAsp = scW / scH;
		float recAsp = (backgroundMediaRes.getWidth()*backgroundMediaRes.getCrop().getWidth()) / (backgroundMediaRes.getHeight()*backgroundMediaRes.getCrop().getHeight());
		if (recAsp > engAsp) {
			outWid = scH * recAsp;
		}

		ViewerCreationArgs bacckyArgs =
			ViewerCreationArgs(backgroundMedia, VIEW_TYPE_TITLED_MEDIA_VIEWER,
							   ci::vec3(mDisplaySize.x / 2.0f, mDisplaySize.y / 2.0f, 0.0f),
							   ViewerCreationArgs::kViewLayerBackground, outWid, true, false, false);
		bacckyArgs.mTouchEvents	   = false;
		bacckyArgs.mLooped		   = true;
		bacckyArgs.mAutoStart	   = true;
		bacckyArgs.mVolume		   = 0;
		bacckyArgs.mAmSlideContent = true;
		addViewer(bacckyArgs);
	}
}

void ViewerController::fullscreenViewer(BaseElement* viewer, const bool immediate, const bool showController) {
	if (!viewer) return;

	if (!mNormalLayer) {
		return;
	}

	auto viewerPos		 = viewer->getPosition();
	auto viewerGlobalPos = viewer->getGlobalPosition();
	viewer->setUnfullscreenRect(
		ci::Rectf(viewerPos.x, viewerPos.y, viewer->getWidth() + viewerPos.x, viewer->getHeight() + viewerPos.y));

	ViewerCreationArgs fullscreenLaunchArgs =
		ViewerCreationArgs(ds::model::ContentModelRef(), VIEW_TYPE_FULLSCREEN_CONTROLLER,
						   ci::vec3(viewerGlobalPos.x + viewer->getWidth() / 2.0f,
									viewerGlobalPos.y + viewer->getHeight() / 2.0f, viewerGlobalPos.z),
						   ViewerCreationArgs::kViewLayerTop, 0.0f, true);

	const float screenWidth	 = mNormalLayer->getWidth();  // mDisplaySize.x;
	const float screenHeight = mNormalLayer->getHeight(); // mDisplaySize.y;
	const float screenAsp	 = screenWidth / screenHeight;

	float viewerAsp	  = viewer->getWidth() / viewer->getHeight();
	float viewerScale = viewer->getScale().x;


	bool didWebSpecial = false;
	if (auto tmv = dynamic_cast<TitledMediaViewer*>(viewer)) {
		if (auto mp = tmv->getMediaPlayer()) {
			if (auto webPlayer = dynamic_cast<ds::ui::WebPlayer*>(mp->getPlayer())) {
				mp->setWebViewSize(ci::vec2(screenWidth, screenHeight));
				mp->setSize(ci::vec2(screenWidth, screenHeight));
				viewer->mContentAspectRatio = screenAsp;
				if (immediate) {
					viewer->setViewerWidth(screenWidth);
					viewer->setPosition(0.0f, 0.0f);
				} else {
					viewer->animateWidthTo(screenWidth);
					viewer->tweenPosition(ci::vec3(0.0f), viewer->getAnimateDuration(), 0.0f, ci::easeInOutQuad);
				}
				didWebSpecial = true;
			}
		}
	}

	if (!didWebSpecial) {
		if (viewerScale == 0.0f) viewerScale = 0.001f;

		if (viewerAsp > screenAsp) {
			if (immediate) {
				viewer->setViewerWidth(screenWidth / viewerScale);
				viewer->setPosition(0.0f, screenHeight / 2.0f - viewer->getHeight() / 2.0f);
			} else {
				viewer->animateWidthTo(screenWidth / viewerScale);
				float finalHeight = screenWidth / viewerAsp;
				viewer->tweenPosition(ci::vec3(0.0f, screenHeight / 2.0f - finalHeight / 2.0f, 0.0f),
									  viewer->getAnimateDuration(), 0.0f, ci::easeInOutQuad);
			}
		} else {
			if (immediate) {
				viewer->setViewerHeight(screenHeight / viewerScale);
				viewer->setPosition(screenWidth / 2.0f - viewer->getScaleWidth() / 2.0f, 0.0f);
			} else {
				viewer->animateHeightTo(screenHeight / viewerScale);
				float finalWidth = screenHeight * viewerAsp;
				viewer->tweenPosition(ci::vec3(screenWidth / 2.0f - finalWidth / 2.0f, 0.0f, 0.0f),
									  viewer->getAnimateDuration(), 0.0f, ci::easeInOutQuad);
			}
		}
	}

	viewer->setIsFullscreen(true);

	ds::ui::Sprite* fullscreenDarkener = nullptr;
	auto			findy			   = mFullscreenDarkeners.find(viewer);
	if (findy != mFullscreenDarkeners.end()) {
		fullscreenDarkener = findy->second;
	}

	if (!fullscreenDarkener) {
		fullscreenDarkener = new ds::ui::Sprite(mEngine, getWidth(), getHeight());
		fullscreenDarkener->setTransparent(false);
		fullscreenDarkener->setColor(ci::Color::black());
		fullscreenDarkener->setOpacity(0.0f);
	}

	if (fullscreenDarkener) {
		if (viewer->getViewerLayer() == ViewerCreationArgs::kViewLayerNormal && mNormalLayer) {
			mNormalLayer->addChildPtr(fullscreenDarkener);
			fullscreenDarkener->setPosition(mNormalLayer->globalToLocal(mTopLayer->getGlobalPosition()));
		} else if (viewer->getViewerLayer() == ViewerCreationArgs::kViewLayerBackground && mBackgroundLayer) {
			mBackgroundLayer->addChildPtr(fullscreenDarkener);
			fullscreenDarkener->setPosition(mBackgroundLayer->globalToLocal(mTopLayer->getGlobalPosition()));
		} else {
			DS_LOG_WARNING("Invalid viewer layer set on a viewer or a layer doesn't exist!");
			fullscreenDarkener->release();
			return;
		}

		fullscreenDarkener->enable(true);
		fullscreenDarkener->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
		fullscreenDarkener->setTapCallback([this](ds::ui::Sprite*, const ci::vec3& pos) {
			mEngine.getNotifier().notify(RequestViewerLaunchEvent(
				ViewerCreationArgs(ds::model::ContentModelRef(), VIEW_TYPE_FULLSCREEN_CONTROLLER, pos,
								   ViewerCreationArgs::kViewLayerTop)));
		});
		fullscreenDarkener->setDoubleTapCallback(
			[this, viewer](ds::ui::Sprite*, const ci::vec3&) { unfullscreenViewer(viewer, false); });
		fullscreenDarkener->tweenOpacity(mEngine.getAppSettings().getFloat("ui:fullscreen_darkener_opacity", 0, 0.8f),
										 viewer->getAnimateDuration());

		mFullscreenDarkeners[viewer] = fullscreenDarkener;
	}

	viewer->activatePanel();

	if (showController) {
		mEngine.getNotifier().notify(RequestViewerLaunchEvent(fullscreenLaunchArgs));
	}
}

void ViewerController::unfullscreenViewer(BaseElement* viewer, const bool immediate) {
	if (!viewer) return;

	if (!mNormalLayer) {
		return;
	}

	const float screenWidth	 = mDisplaySize.x;
	const float screenHeight = mDisplaySize.y;

	removeFullscreenDarkener(viewer);

	ci::Rectf destRect = viewer->getUnfullscreenRect();
	// if this is a large asset, make sure it scales down when un-fullscreening
	if (destRect.getWidth() >= screenWidth || destRect.getHeight() >= screenHeight) {
		ci::vec2 centery = destRect.getCenter();
		if (mEngine.getAppSettings().getString("screen:app:mode", 0, "wall") == "single") {
			centery.x = screenWidth / 2.0f;
			centery.y = screenHeight / 2.0f;
		}

		ci::vec2 defaultSize = viewer->getDefaultSize();

		// do an additional check for 'bigness' on small displays
		if (defaultSize.x > screenWidth / 2.0f || defaultSize.y > screenHeight / 2.0f) {
			defaultSize = viewer->getMinSize();
			/* float waspRatio = screenWidth / screenHeight;
			float daspRatio = defaultSize.x / defaultSize.y;
			// a = w / h; h = w / a; w = a * h
			if (daspRatio > waspRatio) {
				defaultSize.x = screenWidth / 2.0f;
				defaultSize.y = defaultSize.x / daspRatio;
			} else {
				defaultSize.y = screenHeight / 2.0f;
				defaultSize.x = daspRatio * defaultSize.y;
			} */
		}


		destRect.x1 = centery.x - defaultSize.x / 2.0f;
		destRect.x2 = centery.x + defaultSize.x / 2.0f;
		destRect.y1 = centery.y - defaultSize.y / 2.0f;
		destRect.y2 = centery.y + defaultSize.y / 2.0f;
	}

	if (auto tmv = dynamic_cast<TitledMediaViewer*>(viewer)) {
		if (auto mp = tmv->getMediaPlayer()) {
			if (auto webPlayer = dynamic_cast<ds::ui::WebPlayer*>(mp->getPlayer())) {

				auto webSize = mEngine.getWafflesSettings().getVec2("web:default_size", 0, ci::vec2(-1.0f, -1.0f));
				mp->setWebViewSize(webSize);
				mp->setSize(ci::vec2(destRect.getWidth(), destRect.getHeight()));
				viewer->mContentAspectRatio = destRect.getAspectRatio();
			}
		}
		tmv->showTitle();
		tmv->showInnerSideBar();
	}

	if (immediate) {
		viewer->setPosition(destRect.getX1(), destRect.getY1());
		viewer->setViewerWidth(destRect.getWidth());
		viewer->setRotation(0.0f);
	} else {
		viewer->tweenPosition(ci::vec3(destRect.getX1(), destRect.getY1(), 0.0f), viewer->getAnimateDuration(), 0.0f,
							  ci::easeInOutQuad);
		viewer->animateWidthTo(destRect.getWidth());
		viewer->tweenRotation(ci::vec3(0.0f, 0.0f, 0.0f), viewer->getAnimateDuration(), 0.0f, ci::easeInOutQuad);
	}

	viewer->setIsFullscreen(false);
}

void ViewerController::removeFullscreenDarkener(BaseElement* be) {
	if (!be) {
		return;
	}

	auto findy = mFullscreenDarkeners.find(be);
	if (findy != mFullscreenDarkeners.end()) {
		be->setIsFullscreen(false);
		auto darkener = findy->second;
		darkener->enable(false);
		darkener->tweenOpacity(0.0f, be->getAnimateDuration(), 0.0f, ci::easeNone, [darkener] { darkener->release(); });
		mFullscreenDarkeners.erase(be);

		if (mFullscreenDarkeners.empty()) {
			auto fscs = getViewersOfType(VIEW_TYPE_FULLSCREEN_CONTROLLER);
			for (auto it : fscs) {
				animateViewerOff(it, 0.0f, ANIMATE_OFF_SHRINK);
			}
		}
	}
}

} // namespace waffles
