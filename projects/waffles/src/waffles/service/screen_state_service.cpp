#include "stdafx.h"

#include "screen_state_service.h"

#include <fstream>
#include <poco/DateTimeFormatter.h>
#include <poco/DateTimeParser.h>

#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/media/player/pdf_player.h>
#include <ds/ui/media/player/stream_player.h>
#include <ds/ui/media/player/video_player.h>
#include <ds/ui/sprite/pdf.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/video.h>
#include <ds/util/file_meta_data.h>

#include "app/app_defs.h"
#include "waffles/waffles_events.h"
#include "events/state_events.h"
#include "waffles/viewers/base_element.h"
#include "waffles/viewers/titled_media_viewer.h"
#include "waffles/viewers/viewer_controller.h"

namespace {

static const std::string AUTOSAVE_FILE = "%LOCAL%/waffles-neu/autosave_state.xml";
static const std::string SAVED_STATES  = "%LOCAL%/waffles-neu/saved_states.xml";
} // namespace

namespace waffles {

ScreenStateService::ScreenStateService(ds::ui::SpriteEngine& g)
	: mEngine(g)
	, ds::AutoUpdate(g)
	, mEventClient(g)
	, mSaveService(g, []() { return new FileWriteRunnable(); })
	, mAutoSave(false)
	, mLastAutoSave(0)
	, mAutoSaveDuration(5)
	, mInitialized(false) {

	mEventClient.listenToEvents<StateSaveRequest>([this](auto& e) { saveNamedState(e.mStateName); });
	mEventClient.listenToEvents<StateLoadRequest>([this](auto& e) { loadNamedState(e.mStateName); });
	mEventClient.listenToEvents<CmsDataLoadCompleteEvent>([this](auto& e) { initialize(); });
	mEventClient.listenToEvents<StateDeleteRequest>([this](auto& e) { deleteNamedState(e.mStateName); });
}

void ScreenStateService::initialize() {
	if (mInitialized) {
		return;
	}

	mInitialized = true;

	bool allowManualSaves = mEngine.getAppSettings().getBool("screen:state:manual_saves", 0, true);

	try {
		if (allowManualSaves) {
			mSaveStateXml = ci::XmlTree(ci::loadFile(ds::Environment::expand(SAVED_STATES)));
			parseStateMetadata();
		}
	} catch (std::exception&) {
		// this is most likely that the save states file doens't exist, which is fine
	}

	if (!mSaveStateXml.hasChild("states")) {
		mSaveStateXml = ci::XmlTree::createDoc();
		ci::XmlTree statesChild;
		statesChild.setTag("states");
		mSaveStateXml.push_back(statesChild);
	}

	mAutoSave		  = mEngine.getAppSettings().getBool("screen:state:auto_restore", 0, false);
	mAutoSaveDuration = mEngine.getAppSettings().getInt("screen:state:auto_save_seconds", 0, 5);
	if (mAutoSave) {
		restoreAutoSaveState();
	}
}

void ScreenStateService::parseStateMetadata() {
	auto statesModel = mEngine.mContent.getChildByName("states");
	statesModel.clearChildren();
	auto theStates = mSaveStateXml.getChild("states");
	int	 theId	   = 1;
	for (auto it = theStates.begin("state"); it != theStates.end(); ++it) {
		ds::model::ContentModelRef stateInfo;
		if (it->hasAttribute("name")) {
			stateInfo.setProperty("name", it->getAttributeValue<std::string>("name"));
		}
		if (it->hasAttribute("date")) {
			std::string theDate = it->getAttributeValue<std::string>("date");
			stateInfo.setProperty("save_date", theDate);
		}

		int numViewers = (int)it->getChildren().size();
		stateInfo.setProperty("viewer_count", numViewers);
		stateInfo.setId(theId);
		stateInfo.setProperty("is_saved", 1);
		theId++;
		statesModel.addChild(stateInfo);
	}

	mEngine.mContent.replaceChild(statesModel);

	mEngine.getNotifier().notify(StatesUpdatedEvent());
}

void ScreenStateService::autoSaveState() {
	ci::XmlTree theDoc = ci::XmlTree::createDoc();
	ci::XmlTree rooty  = ci::XmlTree();
	rooty.setTag("autosave");

	saveState(rooty);

	theDoc.push_back(rooty);
	mSaveService.start([this, theDoc](FileWriteRunnable& q) { q.setSaveableXml(theDoc, AUTOSAVE_FILE); });
	mLastAutoSave = Poco::Timestamp().epochMicroseconds();
}

void ScreenStateService::saveNamedState(const std::string& stateName) {
	auto&				theStates = mSaveStateXml.getChild("states");
	Poco::LocalDateTime ldt;
	std::string			dateString = Poco::DateTimeFormatter::format(ldt, "%Y-%m-%d %H:%M:%S:%i");

	if (hasNamedState(stateName)) {
		for (auto it = theStates.begin("state"); it != theStates.end(); ++it) {
			if (it->hasAttribute("name") && it->getAttributeValue<std::string>("name") == stateName) {
				ci::XmlTree& oldState	 = *it;
				auto&		 theChildren = oldState.getChildren();
				theChildren.clear();
				oldState.setAttribute("date", dateString);
				saveState(oldState);
				break;
			}
		}

	} else {
		ci::XmlTree newState = ci::XmlTree();
		newState.setTag("state");
		newState.setAttribute("name", stateName);
		newState.setAttribute("date", dateString);
		saveState(newState);
		theStates.push_back(newState);
	}

	parseStateMetadata();
	mSaveService.start([this](FileWriteRunnable& q) { q.setSaveableXml(mSaveStateXml, SAVED_STATES); });
}

void ScreenStateService::saveState(ci::XmlTree& tree) {
	auto vc = ViewerController::getInstance();
	for (auto it : vc->getViewers()) {
		if (it->getViewerType() != VIEW_TYPE_TITLED_MEDIA_VIEWER) continue;
		ci::XmlTree viewerNode = ci::XmlTree();
		viewerNode.setTag("viewer");
		writeViewer(viewerNode, it);
		tree.push_back(viewerNode);
	}
}

void ScreenStateService::writeViewer(ci::XmlTree& theTree, BaseElement* be) {
	if (!be) return;
	if (be->getIsFatalErrored()) return;
	if (be->getIsAboutToBeRemoved()) return;

	auto globalPos = be->getGlobalPosition();
	auto theModel  = be->getMedia();
	auto viewType  = be->getViewerType();
	theTree.setAttribute("id", theModel.getId());
	theTree.setAttribute("viewer_type", viewType);
	theTree.setAttribute("media_type", theModel.getPropertyString("type"));
	theTree.setAttribute("name", theModel.getPropertyString("name"));
	theTree.setAttribute("body", theModel.getPropertyString("body"));
	theTree.setAttribute("width", be->getWidth());
	theTree.setAttribute("x", globalPos.x);
	theTree.setAttribute("y", globalPos.y);
	theTree.setAttribute("fullscreen", be->getIsFullscreen());
	theTree.setAttribute("viewer_layer", be->getViewerLayer());
	theTree.setValue(theModel.getPropertyResource("media_res").getAbsoluteFilePath());


	int	   pdfPage		= 0;
	double videoPos		= 0.0;
	float  videoVol		= 0.0f;
	bool   videoLoop	= true;
	bool   videoPlaying = true;
	bool   videoMuted	= false;
	float  streamWidth	= 1920.0f;
	float  streamHeight = 1080.0f;
	if (viewType == VIEW_TYPE_TITLED_MEDIA_VIEWER) {
		TitledMediaViewer* tmv = dynamic_cast<TitledMediaViewer*>(be);
		if (tmv) {
			auto thePlayer = tmv->getMediaPlayer()->getPlayer();
			auto vidPlayer = dynamic_cast<ds::ui::VideoPlayer*>(thePlayer);
			if (vidPlayer && vidPlayer->getVideo()) {
				auto theVideo = vidPlayer->getVideo();
				videoPos	  = theVideo->getCurrentPosition();
				videoVol	  = theVideo->getVolume();
				videoPlaying  = theVideo->getIsPlaying();
				videoMuted	  = theVideo->getIsMuted();
				videoLoop	  = theVideo->getIsLooping();
			}

			auto strmPlayer = dynamic_cast<ds::ui::StreamPlayer*>(thePlayer);
			if (strmPlayer && strmPlayer->getVideo()) {
				auto theVideo = strmPlayer->getVideo();
				streamWidth	  = theVideo->getWidth();
				streamHeight  = theVideo->getHeight();
			}

			auto pdfPlayer = dynamic_cast<ds::ui::PDFPlayer*>(thePlayer);
			if (pdfPlayer && pdfPlayer->getPDF()) {
				pdfPage = pdfPlayer->getPDF()->getPageNum();
			}
		}
	}

	theTree.setAttribute("pdf_page", pdfPage);
	theTree.setAttribute("video_pos", videoPos);
	theTree.setAttribute("video_vol", videoVol);
	theTree.setAttribute("video_loop", videoLoop);
	theTree.setAttribute("video_playing", videoPlaying);
	theTree.setAttribute("video_muted", videoMuted);
	theTree.setAttribute("stream_width", streamWidth);
	theTree.setAttribute("stream_height", streamHeight);
}

void ScreenStateService::restoreAutoSaveState() {
	try {
		mEngine.getNotifier().notify(RequestCloseAllEvent());
		ci::XmlTree xml(cinder::loadFile(ds::Environment::expand(AUTOSAVE_FILE)));
		auto		autosave = xml.getChild("autosave");
		loadState(autosave);
		mLastAutoSave = Poco::Timestamp().epochMicroseconds();
	} catch (std::exception&) {}
}

bool ScreenStateService::loadNamedState(const std::string& stateName) {
	if (!hasNamedState(stateName)) {
		DS_LOG_WARNING("ScreenStateService couldn't load the state with name " << stateName
																			   << " cause it doesn't exist.");
		return false;
	}

	auto theStates = mSaveStateXml.getChild("states");
	for (auto it = theStates.begin("state"); it != theStates.end(); ++it) {
		if (it->hasAttribute("name") && it->getAttributeValue<std::string>("name") == stateName) {
			mEngine.getNotifier().notify(RequestCloseAllEvent());
			loadState(*it);
			return true;
		}
	}

	return false;
}

void ScreenStateService::deleteNamedState(const std::string& stateName) {
	if (!hasNamedState(stateName)) {
		return;
	}

	ci::XmlTree newDoc	  = ci::XmlTree::createDoc();
	ci::XmlTree newStates = ci::XmlTree();
	newStates.setTag("states");

	auto theStates = mSaveStateXml.getChild("states");
	for (auto it = theStates.begin("state"); it != theStates.end(); ++it) {
		if (it->hasAttribute("name") && it->getAttributeValue<std::string>("name") == stateName) {
			// ignore the named one
		} else {
			newStates.push_back(*it);
		}
	}
	newDoc.push_back(newStates);
	mSaveStateXml = newDoc;
	parseStateMetadata();
	mSaveService.start([this](FileWriteRunnable& q) { q.setSaveableXml(mSaveStateXml, SAVED_STATES); });
}

bool ScreenStateService::hasNamedState(const std::string& stateName) {
	auto theStates = mSaveStateXml.getChild("states");
	for (auto it = theStates.begin("state"); it != theStates.end(); ++it) {
		if (it->hasAttribute("name") && it->getAttributeValue<std::string>("name") == stateName) {
			return true;
		}
	}
	return false;
}

void ScreenStateService::loadState(ci::XmlTree& xml) {
	for (auto xit = xml.begin("viewer"); xit != xml.end(); ++xit) {
		try {
			readMediaItem((*xit));
		} catch (std::exception& e) {
			DS_LOG_WARNING(
				"Didn't read a media item back in when reading state! Probably didn't have a correct attribute. "
				<< e.what());
		}
	}
}

void ScreenStateService::readMediaItem(ci::XmlTree& theTree) {
	if (theTree.getTag() != "viewer") {
		return;
	}
	int			theId		 = theTree.getAttributeValue<int>("id");
	std::string viewType	 = theTree.getAttributeValue<std::string>("viewer_type");
	std::string mediaType	 = theTree.getAttributeValue<std::string>("media_type");
	std::string theTitle	 = theTree.getAttributeValue<std::string>("name");
	std::string theBody		 = theTree.getAttributeValue<std::string>("body");
	float		theWidth	 = theTree.getAttributeValue<float>("width");
	float		theX		 = theTree.getAttributeValue<float>("x");
	float		theY		 = theTree.getAttributeValue<float>("y");
	int			isFullscreen = theTree.getAttributeValue<int>("fullscreen");
	int			viewLayer	 = theTree.getAttributeValue<int>("viewer_layer");

	int	   pdfPage		= theTree.getAttributeValue<int>("pdf_page");
	double videoPos		= theTree.getAttributeValue<double>("video_pos");
	float  videoVol		= theTree.getAttributeValue<float>("video_vol");
	bool   videoLoop	= theTree.getAttributeValue<bool>("video_loop");
	bool   videoPlaying = theTree.getAttributeValue<bool>("video_playing");
	bool   videoMuted	= theTree.getAttributeValue<bool>("video_muted");

	float streamWidth  = 1920.0f;
	float streamHeight = 1080.0f;

	if (theTree.hasAttribute("stream_width")) streamWidth = theTree.getAttributeValue<float>("stream_width");
	if (theTree.hasAttribute("stream_height")) streamHeight = theTree.getAttributeValue<float>("stream_height");

	std::string filePath = theTree.getValue();

	ViewerCreationArgs vca;
	vca.mFromCenter		   = false;
	vca.mCheckBounds	   = true;
	vca.mFullscreen		   = isFullscreen == 1;
	vca.mLocation.x		   = theX;
	vca.mLocation.y		   = theY;
	vca.mLocation.z		   = 0.0f;
	vca.mStartWidth		   = theWidth * mEngine.getAppSettings().getFloat("viewer:master_scale", 0, 1.0f);
	vca.mViewType		   = viewType;
	vca.mViewLayer		   = viewLayer;
	vca.mLooped			   = videoLoop;
	vca.mVolume			   = (int)(videoVol * 100.0f);
	vca.mPage			   = pdfPage;
	vca.mVideoTimePosition = videoPos;
	vca.mAutoStart		   = videoPlaying;
	vca.mMuted			   = videoMuted;
	vca.mMediaRef.setProperty("name", theTitle);
	vca.mMediaRef.setProperty("body", theBody);

	if (viewType == VIEW_TYPE_TITLED_MEDIA_VIEWER) {
		if (mediaType == MEDIA_TYPE_FILE_CMS) {
			ds::model::ContentModelRef theMedia =
				mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", theId);
			vca.mMediaRef = theMedia;
		} else {
			ds::Resource theResource = ds::Resource(filePath);

			if (theResource.getType() == ds::Resource::VIDEO_STREAM_TYPE) {
				theResource.setWidth(streamWidth);
				theResource.setHeight(streamHeight);
			}

			vca.mMediaRef.setPropertyResource("media_res", theResource);
		}
	}

	mEngine.getNotifier().notify(RequestViewerLaunchEvent(vca));
}


void ScreenStateService::update(const ds::UpdateParams& p) {
	if (mAutoSave) {
		Poco::Timestamp::TimeVal nowwy = Poco::Timestamp().epochMicroseconds();
		if ((int)((nowwy - mLastAutoSave) / 1000000) > mAutoSaveDuration) {
			autoSaveState();
		}
	}
}

ScreenStateService::FileWriteRunnable::FileWriteRunnable() {
}

void ScreenStateService::FileWriteRunnable::setSaveableXml(const ci::XmlTree& theData, std::string saveFileName) {
	mData		  = theData;
	mSaveFileName = saveFileName;
}

void ScreenStateService::FileWriteRunnable::run() {
	mData.write(ci::writeFile(ds::Environment::expand(mSaveFileName), true));
}
} // namespace waffles
