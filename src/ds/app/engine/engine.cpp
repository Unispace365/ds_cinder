#include "stdafx.h"

#include "ds/app/engine/engine.h"
#include "ds/app/app.h"
#include "ds/app/auto_draw.h"
#include "ds/app/environment.h"
#include "ds/app/engine/engine_roots.h"
#include "ds/app/engine/engine_service.h"
#include "ds/app/engine/engine_stats_view.h"
#include "ds/app/error.h"
#include "ds/cfg/settings.h"
#include "ds/cfg/settings_editor.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/math/math_defs.h"
#include "ds/ui/ip/ip_defs.h"
#include "ds/ui/ip/functions/ip_circle_mask.h"
#include "ds/ui/touch/draw_touch_view.h"
#include "ds/ui/touch/touch_event.h"
#include "ds/util/file_meta_data.h"

#include <cinder/Display.h>
#include <boost/algorithm/string.hpp>

//! This entire header is included for one single
//! function Poco::Path::expand. This slowly needs
//! to get removed. Poco is not part of the Cinder.
#include <Poco/Path.h>

//#include <boost/algorithm/string/predicate.hpp>

#pragma warning (disable : 4355)    // disable 'this': used in base member initializer list

namespace {
void				root_setup(std::vector<std::unique_ptr<ds::EngineRoot>>&);
}

const ds::BitMask	ds::ENGINE_LOG = ds::Logger::newModule("engine");

namespace ds {

const int Engine::NumberOfNetworkThreads = 2;

Engine::Engine(	ds::App& app, ds::EngineSettings &settings,
				ds::EngineData& ed, const RootList& _roots)
	: ds::ui::SpriteEngine(ed)
	, mTweenline(app.timeline())
	, mIdling(true)
	, mTouchMode(ds::ui::TouchMode::kTuioAndMouse)
	, mTouchManager(*this, mTouchMode)
	, mPangoFontService(*this)
	, mSettings(settings)
	, mSettingsEditor(nullptr)
	, mTouchBeginEvents(mTouchMutex,	mLastTouchTime, mIdling, [&app, this](const ds::ui::TouchEvent& e) {app.onTouchesBegan(e); this->mTouchManager.touchesBegin(e);}, "touchbegin")
	, mTouchMovedEvents(mTouchMutex,	mLastTouchTime, mIdling, [&app, this](const ds::ui::TouchEvent& e) {app.onTouchesMoved(e); this->mTouchManager.touchesMoved(e);}, "touchmoved")
	, mTouchEndedEvents(mTouchMutex,	mLastTouchTime, mIdling, [&app, this](const ds::ui::TouchEvent& e) {app.onTouchesEnded(e); this->mTouchManager.touchesEnded(e);}, "touchend")
	, mMouseBeginEvents(mTouchMutex,	mLastTouchTime, mIdling, [this](const MousePair& e)  {handleMouseTouchBegin(e.first, e.second);}, "mousebegin")
	, mMouseMovedEvents(mTouchMutex,	mLastTouchTime, mIdling, [this](const MousePair& e)  {handleMouseTouchMoved(e.first, e.second);}, "mousemoved")
	, mMouseEndedEvents(mTouchMutex,	mLastTouchTime, mIdling, [this](const MousePair& e)  {handleMouseTouchEnded(e.first, e.second);}, "mouseend")
	, mTuioObjectsBegin(mTouchMutex,	mLastTouchTime, mIdling, [&app](const TuioObject& e) {app.tuioObjectBegan(e);}, "tuiobegin")
	, mTuioObjectsMoved(mTouchMutex,	mLastTouchTime, mIdling, [&app](const TuioObject& e) {app.tuioObjectMoved(e);}, "tuiomoved")
	, mTuioObjectsEnded(mTouchMutex,	mLastTouchTime, mIdling, [&app](const TuioObject& e) {app.tuioObjectEnded(e);}, "tuioend")
	, mHideMouse(false)
	, mUniqueColor(0, 0, 0)
	, mAutoDraw(new AutoDrawService())
	, mCachedWindowW(0)
	, mCachedWindowH(0)
	, mAverageFps(0.0f)
	, mFonts(*this)
{
	addChannel(ERROR_CHANNEL, "A master list of all errors in the system.");
	addService("ds/error", *(new ErrorService(*this)));


	// For now, install some default image processing functions here, for convenience. These are
	// so lightweight it probably makes sense just to have them always available for clients instead
	// of requiring some sort of configuration.
	mIpFunctions.add(ds::ui::ip::CIRCLE_MASK, ds::ui::ip::FunctionRef(new ds::ui::ip::CircleMask()));

	if (mAutoDraw) addService("AUTODRAW", *mAutoDraw);

	ds::Logger::setup(settings);

	// touch settings
	mTouchMode = ds::ui::TouchMode::fromSettings(settings);
	setTouchMode(mTouchMode);
	mTouchManager.setOverrideTranslation(settings.getBool("touch_overlay:override_translation", 0, false));
	mTouchManager.setOverrideDimensions(settings.getVec2("touch_overlay:dimensions", 0, ci::vec2(1920.0f, 1080.0f)));
	mTouchManager.setOverrideOffset(settings.getVec2("touch_overlay:offset", 0, ci::vec2(0.0f, 0.0f)));
	mTouchManager.setTouchFilterRect(settings.getRect("touch_overlay:filter_rect", 0, ci::Rectf(0.0f, 0.0f, 0.0f, 0.0f)));

	mData.mAppInstanceName = settings.getString("platform:guid", 0, "Downstream");


	// don't lose idle just because we got a marker moved event
	mTuioObjectsMoved.setAutoIdleReset(false);

	const bool drawTouches = settings.getBool("touch_overlay:debug", 0, false);
	mData.mMinTapDistance = settings.getFloat("tap_threshold", 0, 30.0f);
	mData.mMinTouchDistance = settings.getFloat("touch:minimum_distance", 0, 10.0f);
	mData.mSwipeQueueSize = settings.getInt("touch:swipe:queue_size", 0, 4);
	mData.mSwipeMinVelocity = settings.getFloat("touch:swipe:minimum_velocity", 0, 800.0f);
	mData.mSwipeMaxTime = settings.getFloat("touch:swipe:maximum_time", 0, 0.5f);
	mData.mFrameRate = settings.getFloat("frame_rate", 0, 60.0f);

	const bool verboseTouchLogging = settings.getBool("touch_overlay:verbose_logging", 0, false);
	mTouchManager.setVerboseLogging(verboseTouchLogging);

	mData.mWorldSize = settings.getVec2("world_dimensions", 0, ci::vec2(0.0f, 0.0f));

	// Src rect and dst rect are new, and should obsolete local_rect. For now, default to illegal values,
	// which makes them get ignored.
	if (settings.hasSetting("src_rect") || settings.hasSetting("dst_rect")) {

		const ci::Rectf		empty_rect(0.0f, 0.0f, 0.0f, 0.0f);

		mData.mSrcRect = settings.getRect("src_rect", 0, empty_rect);
		mData.mDstRect = settings.getRect("dst_rect", 0, empty_rect);
		
	}

	if(mData.mDstRect.getWidth() < 1 || mData.mDstRect.getHeight() < 1){
		DS_LOG_WARNING("Screen rect is 0 width or height. Overriding to full screen size");
		ci::DisplayRef mainDisplay = ci::Display::getMainDisplay();
		ci::Rectf mainDisplayRect = ci::Rectf(0.0f, 0.0f, (float)mainDisplay->getWidth(), (float)mainDisplay->getHeight());
		mData.mSrcRect = mainDisplayRect;
		mData.mDstRect = mainDisplayRect;
		if(mData.mWorldSize.x < 1 || mData.mWorldSize.y < 1){
			mData.mWorldSize = ci::vec2(mainDisplayRect.getWidth(), mainDisplayRect.getHeight());
		}
	}

	DS_LOG_INFO("Screen dst_rect is (" << mData.mDstRect.x1 << ", " << mData.mDstRect.y1 << ") - (" << mData.mDstRect.x2 << ", " << mData.mDstRect.y2 << ")");

	// Don't construct roots on startup for clients, and instead create them when we connect to a server
	const std::string	arch(settings.getString("platform:architecture", 0, ""));
	bool isClient = false;
	if(arch == "client") isClient = true;

	sprite_id_t							root_id = EMPTY_SPRITE_ID - 1;
	// Construct the root sprites
	if(!isClient){
		RootList				roots(_roots.runInitFn());
		if(roots.empty()) roots.ortho();
		for(auto it = roots.mRoots.begin(), end = roots.mRoots.end(); it != end; ++it) {
			RootList::Root&			r(*it);
			r.mRootId = root_id;
			std::unique_ptr<EngineRoot>		root;
			if(r.mType == r.kOrtho) root.reset(new OrthRoot(*this, r, r.mRootId));
			else if(r.mType == r.kPerspective) root.reset(new PerspRoot(*this, r, r.mRootId, r.mPersp));
			if(!root){
				DS_LOG_WARNING("Couldn't create root in the engine!");
				continue;
			}
			mRoots.push_back(std::move(root));
			--root_id;
		}
		if(mRoots.empty()) {
			DS_LOG_WARNING("Engine can't create single root");
		}
		root_setup(mRoots);
	}

	// If we're drawing the touches, create a separate top-level root to do that
	// For clients, the debug touch root is created by the server and synced

	if (drawTouches && !isClient) {
		RootList::Root					root_cfg;
		root_cfg.mType = root_cfg.kOrtho;
		root_cfg.mDebugDraw = true;
		root_cfg.mDrawScaled = true;
		root_cfg.mRootId = root_id;
		std::unique_ptr<EngineRoot>		root;
		root.reset(new OrthRoot(*this, root_cfg, root_id));
		if (root) {
			ds::ui::Sprite*				parent = root->getSprite();
			if (parent) {
				parent->setDrawDebug(true);
				mRoots.push_back(std::move(root));
				
			}
		}
	}
	// Add a view for displaying the stats.
	createStatsView(root_id);

	// Initialize the roots
	const EngineRoot::Settings	er_settings(mData.mWorldSize, mData.mSrcRect, mData.mDstRect);
	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		EngineRoot&				r(*(it->get()));
		r.setup(er_settings);
	}

	mRotateTouchesDefault = settings.getBool("touch:rotate_touches_default", 0, false);

	// SETUP RESOURCES
	std::string resourceLocation = ds::getNormalizedPath(settings.getString("resource_location", 0, ""));
	if (resourceLocation.empty()) {
		// This is valid, though unusual
		std::cout << "Engine() has no resource_location setting" << std::endl;
	} else {
		if (boost::contains(resourceLocation, "%USERPROFILE%")) {
			DS_LOG_WARNING("Using \"%USERPROFILE%\" in a resource_location path is deprecated.  You probably want \"%LOCAL%\"...");
#ifndef _WIN32
			boost::replace_all(resourceLocation, "%USERPROFILE%", Poco::Path::expand("~"));
			DS_LOG_WARNING("Linux workaround: Converting \"%USERPROFILE%\" to \"~\" in resources_location...");
#endif
		}

		resourceLocation = Poco::Path::expand(resourceLocation);
		resourceLocation = ds::Environment::expand(resourceLocation); // allow use of %APP%, etc
		Resource::Id::setupPaths(
			ds::getNormalizedPath(resourceLocation),
			ds::getNormalizedPath(settings.getString("resource_db", 0)),
			ds::getNormalizedPath(settings.getString("project_path", 0))
		);
	}

	setIdleTimeout((int)settings.getFloat("idle_time", 0, 300));
	setMute(settings.getBool("platform:mute", 0, false));
}


void Engine::prepareSettings(ci::app::AppBase::Settings& settings){
	std::string screenMode = "window";

	settings.setWindowSize(static_cast<int>(getWidth()), static_cast<int>(getHeight()));
	if(mSettings.getUsingDefault()){
		screenMode = "borderless";
	}
	screenMode = mSettings.getString("screen:mode", 0, screenMode);
	if(screenMode == "full"){
		settings.setFullScreen(true);
	} else if(screenMode == "fullscreen"){
		settings.setFullScreen(true);
	} else if(screenMode == "borderless"){
		settings.setBorderless(true);
	}

	settings.setAlwaysOnTop(mSettings.getBool("screen:always_on_top", 0, false));
	
	DS_LOG_INFO("Engine::prepareSettings: screenMode is " << screenMode << " and always on top " << settings.isAlwaysOnTop());

	settings.setResizable(false);


	mHideMouse = mSettings.getBool("hide_mouse", 0, mHideMouse);
	mTuioPort = mSettings.getInt("tuio_port", 0, 3333);
	setTouchSmoothing(mSettings.getBool("touch_smoothing", 0, true));
	setTouchSmoothFrames(mSettings.getInt("touch_smooth_frames", 0, 5));

	settings.setFrameRate(mData.mFrameRate);

	const std::string     nope = "ds:IllegalTitle";
	const std::string     title = mSettings.getString("screen:title", 0, nope);
	if(title != nope) settings.setTitle(title);

}

void Engine::showSettingsEditor(ds::cfg::Settings& theSettings){
	if(mSettingsEditor){
		mSettingsEditor->show();
		mSettingsEditor->showSettings(&theSettings);
	}
}

void Engine::hideSettingsEditor(){
	if(mSettingsEditor){
		mSettingsEditor->hide();
	}
}

bool Engine::isShowingSettingsEditor(){
	if(mSettingsEditor){
		return mSettingsEditor->visible();
	}

	return false;
}

void Engine::setup(ds::App& app) {

	mCinderWindow = app.getWindow();
	//mCinderWindow->spanAllDisplays

	mTouchTranslator.setTranslation(mData.mSrcRect.x1, mData.mSrcRect.y1);
	mTouchTranslator.setScale(mData.mSrcRect.getWidth() / ci::app::getWindowWidth(), mData.mSrcRect.getHeight() / ci::app::getWindowHeight());


	const std::string	arch(mSettings.getString("platform:architecture", 0, ""));
	bool isClient = false;
	if(arch == "client") isClient = true;
	const bool			drawTouches = mSettings.getBool("touch_overlay:debug", 0, false);
	for(auto it = mRoots.begin(), end = mRoots.end(); it != end; ++it) {
		(*it)->postAppSetup();
		(*it)->setCinderCamera();

		// Assume only one debug synchronized root? oh boy I hope so!
		if(!isClient && drawTouches && (*it)->getBuilder().mDebugDraw && (*it)->getBuilder().mSyncronize){

			ds::ui::DrawTouchView* v = new ds::ui::DrawTouchView(*this, mSettings, mTouchManager);
			(*it)->getSprite()->addChildPtr(v);
		}
	}

	const int		w = static_cast<int>(getWidth()),
		h = static_cast<int>(getHeight());
	if(w < 1 || h < 1) {
		// GN: recent updates should make this impossible to get to.
		//		but leaving this here in case some weird case sets the size to an invalid value
		DS_LOG_WARNING("Engine::setup() on 0 size width or height");
	}

	float curr = static_cast<float>(ci::app::getElapsedSeconds());
	mLastTime = curr;
	mLastTouchTime = 0;

	mUpdateParams.setDeltaTime(0.0f);
	mUpdateParams.setElapsedTime(curr);

	// Start any library services
	if(!mData.mServices.empty()) {
		for(auto it = mData.mServices.begin(), end = mData.mServices.end(); it != end; ++it) {
			if(it->second) it->second->start();
		}
	}
}

void Engine::setupTouch(ds::App& a) {
	
	if(ds::ui::TouchMode::hasTuio(mTouchMode)) {
		ci::tuio::Client&		tuioClient = getTuioClient();
		tuioClient.registerTouches(&a);
		registerForTuioObjects(tuioClient);
		try{
			tuioClient.connect(mTuioPort);
			DS_LOG_INFO("TUIO Connected on port " << mTuioPort);
		} catch(std::exception ex) {
			DS_LOG_WARNING("TUIO client could not be started on port " << mTuioPort << ". The most common cause is that the port is already bound by another app.");
		}
	}
}

void Engine::clearRoots(){
	mRoots.clear();
}

void Engine::createClientRoots(std::vector<RootList::Root> roots){
	sprite_id_t	root_id = 0;

	for(auto it = roots.begin(), end = roots.end(); it != end; ++it) {
		const RootList::Root&			r(*it);
		std::unique_ptr<EngineRoot>		root;
		sprite_id_t thisRootId = (*it).mRootId;
		if(r.mType == r.kOrtho) root.reset(new OrthRoot(*this, r, thisRootId));
		else if(r.mType == r.kPerspective) root.reset(new PerspRoot(*this, r, thisRootId, r.mPersp));
		if(!root){
			DS_LOG_WARNING("Engine can't create root");
			continue;
		}
		mRoots.push_back(std::move(root));
		if(thisRootId < root_id){
			root_id = thisRootId;
		}
	}

	--root_id;
	createStatsView(root_id);
	root_setup(mRoots);

	const EngineRoot::Settings	er_settings(mData.mWorldSize, mData.mSrcRect, mData.mDstRect);
	for(auto it = mRoots.begin(), end = mRoots.end(); it != end; ++it) {
		EngineRoot&				r(*(it->get()));
		r.setup(er_settings);
	}

	for(auto it = mRoots.begin(), end = mRoots.end(); it != end; ++it) {
		(*it)->postAppSetup();
		(*it)->setCinderCamera();
	}
}

void Engine::createStatsView(sprite_id_t root_id){
	RootList::Root					root_cfg;
	root_cfg.mType = root_cfg.kOrtho;
	root_cfg.mDebugDraw = true;
	root_cfg.mDrawScaled = false;
	root_cfg.mSyncronize = false;
	std::unique_ptr<EngineRoot>		root;
	root.reset(new OrthRoot(*this, root_cfg, root_id));
	if(root) {
		ds::ui::Sprite*				parent = root->getSprite();
		if(parent) {
			parent->setDrawDebug(true);
			EngineStatsView*		v = new EngineStatsView(*this);
			if(v) {
				parent->addChild(*v);
			}

			mSettingsEditor = new ds::cfg::SettingsEditor(*this);
			if(mSettingsEditor){
				parent->addChildPtr(mSettingsEditor);
			}
			mRoots.push_back(std::move(root));
		}
	}

}

Engine::~Engine() {
	mTuio.disconnect();

	// Important to do this here before the auto update list is destructed.
	// so any autoupdate services get removed.
	mData.clearServices();
}

ds::EventNotifier& Engine::getChannel(const std::string &name) {
	if(name.empty()){
		DS_LOG_WARNING("Engine::getChannel() on empty name");
		if(mChannels.empty()){
			mChannels["blank"] = Channel();
			return mChannels["blank"].mNotifier;
		} else {
			return mChannels.begin()->second.mNotifier;
		}
	}
	if (!mChannels.empty()) {
		auto f = mChannels.find(name);
		if (f != mChannels.end()) return f->second.mNotifier;
	}
	mChannels[name] = Channel();
	auto f = mChannels.find(name);
	if (f != mChannels.end()) return f->second.mNotifier;
	DS_LOG_WARNING("Engine::getChannel() no channel named " + name);
	if(mChannels.empty()){
		mChannels["blank"] = Channel();
		return mChannels["blank"].mNotifier;
	} else {
		return mChannels.begin()->second.mNotifier;
	}
}

void Engine::addChannel(const std::string &name, const std::string &description) {
	if(name.empty()){
		DS_LOG_WARNING("Engine::addChannel() on empty name");
		return;
	}
	mChannels[name] = Channel(description);
}

ds::AutoUpdateList& Engine::getAutoUpdateList(const int mask) {
	if ((mask&AutoUpdateType::SERVER) != 0) return mAutoUpdateServer;
	if ((mask&AutoUpdateType::CLIENT) != 0) return mAutoUpdateClient;
	DS_LOG_WARNING("Engine::getAutoUpdateList() on illegal param");
	return mAutoUpdateServer;
}

void Engine::addService(const std::string& str, ds::EngineService& service) {
	if (mData.mServices.empty()) {
		mData.mServices[str] = &service;
	} else {
		auto found = mData.mServices.find(str);
		if (found != mData.mServices.end()) {
			delete found->second;
			found->second = nullptr;
		}
		mData.mServices[str] = &service;
	}
}

void Engine::addIp(const std::string& key, const ds::ui::ip::FunctionRef& fn) {
	mIpFunctions.add(key, fn);
}

void Engine::loadSettings(const std::string& name, const std::string& filename) {
	mData.mEngineCfg.loadSettings(name, filename);
}

void Engine::saveSettings(const std::string& name, const std::string& filename) {
	mData.mEngineCfg.saveSettings(name, filename);
}

void Engine::appendSettings(const std::string& name, const std::string& filename) {
	mData.mEngineCfg.appendSettings(name, filename);
}

void Engine::loadTextCfg(const std::string& filename) {
	mData.mEngineCfg.loadText(filename, *this);
}

size_t Engine::getRootCount() const {
	return mRoots.size();
}

ui::Sprite& Engine::getRootSprite(const size_t index) {
	if(index < 0 || index >= mRoots.size()){
		DS_LOG_WARNING("Engine::getRootSprite() on invalid index " << index);
		ui::Sprite* fs = mRoots.front()->getSprite();
		return *fs;
	}
	ui::Sprite*		s = mRoots[index]->getSprite();
	if(!s) DS_LOG_WARNING("Engine::getRootSprite() on null sprite");
	return *s;
}

const RootList::Root& Engine::getRootBuilder(const size_t index){
	if(index < 0 || index >= mRoots.size()){
		DS_LOG_WARNING("Engine::getRootBuilder() on invalid index " << index);
		return mRoots.front()->getBuilder();;
	}
	return mRoots[index]->getBuilder();
}

void Engine::updateClient() {
	float curr = static_cast<float>(ci::app::getElapsedSeconds());
	float dt = curr - mLastTime;
	mLastTime = curr;

	if (!mIdling && (curr - mLastTouchTime) >= (float)getIdleTimeout()) {
		mIdling = true;
	}
	{
		std::lock_guard<std::mutex> lock(mTouchMutex);
		mMouseBeginEvents.lockedUpdate();
		mMouseMovedEvents.lockedUpdate();
		mMouseEndedEvents.lockedUpdate();
	}

	mMouseBeginEvents.update(curr);
	mMouseMovedEvents.update(curr);
	mMouseEndedEvents.update(curr);

	mUpdateParams.setDeltaTime(dt);
	mUpdateParams.setElapsedTime(curr);

	mAutoUpdateClient.update(mUpdateParams);

	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		(*it)->updateClient(mUpdateParams);
	}
}

void Engine::updateServer() {
	if(mCachedWindowW != ci::app::getWindowWidth() || mCachedWindowH != ci::app::getWindowHeight()) {
		mCachedWindowW = ci::app::getWindowWidth();
		mCachedWindowH = ci::app::getWindowHeight();
		mTouchTranslator.setScale(	mData.mSrcRect.getWidth() / static_cast<float>(mCachedWindowW),
									mData.mSrcRect.getHeight() / static_cast<float>(mCachedWindowH));
	}

	const float		curr = static_cast<float>(ci::app::getElapsedSeconds());
	const float		dt = curr - mLastTime;
	mLastTime = curr;

	//////////////////////////////////////////////////////////////////////////
	{
		std::lock_guard<std::mutex> lock(mTouchMutex);
		mMouseBeginEvents.lockedUpdate();
		mMouseMovedEvents.lockedUpdate();
		mMouseEndedEvents.lockedUpdate();

		mTouchBeginEvents.lockedUpdate();
		mTouchMovedEvents.lockedUpdate();
		mTouchEndedEvents.lockedUpdate();

		mTuioObjectsBegin.lockedUpdate();
		mTuioObjectsMoved.lockedUpdate();
		mTuioObjectsEnded.lockedUpdate();
	} // unlock touch mutex
	//////////////////////////////////////////////////////////////////////////

	mMouseBeginEvents.update(curr);
	mMouseMovedEvents.update(curr);
	mMouseEndedEvents.update(curr);

	mTouchBeginEvents.update(curr);
	mTouchMovedEvents.update(curr);
	mTouchEndedEvents.update(curr);

	mTuioObjectsBegin.update(curr);
	mTuioObjectsMoved.update(curr);
	mTuioObjectsEnded.update(curr);

	if (!mIdling && (curr - mLastTouchTime) >= (float)getIdleTimeout()) {
		mIdling = true;
	}

	mUpdateParams.setDeltaTime(dt);
	mUpdateParams.setElapsedTime(curr);

	mAutoUpdateServer.update(mUpdateParams);

	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		(*it)->updateServer(mUpdateParams);
	}
}

void Engine::markCameraDirty() {
	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		(*it)->markCameraDirty();
	}
}

PerspCameraParams Engine::getPerspectiveCamera(const size_t index) const {
	const PerspRoot*			root = nullptr;
	if (index < mRoots.size()) root = dynamic_cast<const PerspRoot*>(mRoots[index].get());
	if (root) {
		return root->getCamera();
	}
	DS_LOG_ERROR(" Engine::getPerspectiveCamera() on invalid root (" << index << ")");
	return PerspCameraParams();
}

const ci::CameraPersp& Engine::getPerspectiveCameraRef(const size_t index) const {
	const PerspRoot*			root = nullptr;
	if (index < mRoots.size()) root = dynamic_cast<const PerspRoot*>(mRoots[index].get());
	if (root) {
		return root->getCameraRef();
	} 
	DS_LOG_ERROR(" Engine::getPerspectiveCamera() on invalid root (" << index << ")");
	static ci::CameraPersp defaultCamera;
	return defaultCamera;
}

void Engine::setPerspectiveCamera(const size_t index, const PerspCameraParams& p) {
	PerspRoot*					root = nullptr;
	if (index < mRoots.size()) root = dynamic_cast<PerspRoot*>(mRoots[index].get());
	if (root) {
		root->setCamera(p);
	} else {
		DS_LOG_ERROR(" Engine::setPerspectiveCamera() on invalid root (" << index << ")");
	}
}

void Engine::setPerspectiveCameraRef(const size_t index, const ci::CameraPersp& p){
	PerspRoot*					root = nullptr;
	if(index < mRoots.size()) root = dynamic_cast<PerspRoot*>(mRoots[index].get());
	if(root) {
		root->setCameraRef(p);
	} else {
		DS_LOG_ERROR(" Engine::setPerspectiveCameraRef() on invalid root (" << index << ")");
	}
}

float Engine::getOrthoFarPlane(const size_t index)const{
	const OrthRoot*				root = nullptr;
	if(index < mRoots.size()) root = dynamic_cast<const OrthRoot*>(mRoots[index].get());
	if(root) {
		return root->getFarPlane();
	}
	DS_LOG_ERROR(" Engine::getOrthoFarPlane() on invalid root (" << index << ")");

	return 0.0f;
}

float Engine::getOrthoNearPlane(const size_t index)const{
	const OrthRoot*				root = nullptr;
	if(index < mRoots.size()) root = dynamic_cast<const OrthRoot*>(mRoots[index].get());
	if(root) {
		return root->getNearPlane();
	}
	DS_LOG_ERROR(" Engine::getOrthoNearPlane() on invalid root (" << index << ")");
	return 0.0f;
}

void Engine::setOrthoViewPlanes(const size_t index, const float nearPlane, const float farPlane){
	OrthRoot*				root = nullptr;
	if(index < mRoots.size()) root = dynamic_cast<OrthRoot*>(mRoots[index].get());
	if(root) {
		root->setViewPlanes(nearPlane, farPlane);
		root->setCinderCamera();
		return;
	}
	DS_LOG_ERROR(" Engine::setOrthoViewPlanes() on invalid root (" << index << ")");
}
void Engine::clearAllSprites(const bool clearDebug) {
	for (auto it=mRoots.begin(), end=mRoots.end(); it != end; ++it) {
		if((*it)->getBuilder().mDebugDraw && !clearDebug) continue;
		(*it)->clearChildren();
	}
}

void Engine::registerForTuioObjects(ci::tuio::Client& client) {
	if (mSettings.getBool("tuio:receive_objects", 0, false)) {
		client.registerObjectAdded([this](ci::tuio::Object o) { this->mTuioObjectsBegin.incoming(TuioObject(o.getFiducialId(), o.getPos(), o.getAngle())); });
		client.registerObjectUpdated([this](ci::tuio::Object o) { this->mTuioObjectsMoved.incoming(TuioObject(o.getFiducialId(), o.getPos(), o.getAngle(), o.getSpeed(), o.getRotationSpeed())); });
		client.registerObjectRemoved([this](ci::tuio::Object o) { this->mTuioObjectsEnded.incoming(TuioObject(o.getFiducialId(), o.getPos(), o.getAngle())); });
	}
}

void Engine::drawClient() {
	ci::gl::enableAlphaBlending();

	ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));

	for(auto it = getRoots().begin(), end = getRoots().end(); it != end; ++it){
		(*it)->drawClient(getDrawParams(), getAutoDrawService());
	}
}

void Engine::drawServer() {
	ci::gl::enableAlphaBlending();

	ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));

	for(auto it = getRoots().cbegin(), end = getRoots().cend(); it != end; ++it){
		(*it)->drawServer(getDrawParams());
	}
}

ds::sprite_id_t Engine::nextSpriteId() {
	static ds::sprite_id_t              ID = 0;
	++ID;
	// Skip negative values.
	if (ID <= EMPTY_SPRITE_ID) ID = EMPTY_SPRITE_ID + 1;
	return ID;
}

void Engine::registerSprite(ds::ui::Sprite& s) {
	if (s.getId() == ds::EMPTY_SPRITE_ID) {
		DS_LOG_WARNING_M("Engine::registerSprite() on empty sprite ID", ds::ENGINE_LOG);
		assert(false);
		return;
	}
	mSprites[s.getId()] = &s;
}

void Engine::unregisterSprite(ds::ui::Sprite& s) {
	if (mSprites.empty()) return;
	if (s.getId() == ds::EMPTY_SPRITE_ID) {
		DS_LOG_WARNING_M("Engine::unregisterSprite() on empty sprite ID", ds::ENGINE_LOG);
		assert(false);
		return;
	}
	auto it = mSprites.find(s.getId());
	if (it != mSprites.end()) mSprites.erase(it);
}

ds::ui::Sprite* Engine::findSprite(const ds::sprite_id_t id) {
	if (mSprites.empty()) return nullptr;
	auto it = mSprites.find(id);
	if (it == mSprites.end()) return nullptr;
	return it->second;
}

void Engine::spriteDeleted(const ds::sprite_id_t&) {
	// Only a server cares about this; everything else just
	// deletes in-place.
}

ci::Color8u Engine::getUniqueColor() {
	int32_t			i = (mUniqueColor.r << 16) | (mUniqueColor.g << 8) | mUniqueColor.b;
	++i;
	mUniqueColor.r = (i>>16)&0xff;
	mUniqueColor.g = (i>>8)&0xff;
	mUniqueColor.b = (i)&0xff;
	return mUniqueColor;
}

void Engine::touchesBegin(const ds::ui::TouchEvent &e) {
	mTouchBeginEvents.incoming(mTouchTranslator.toWorldSpace(e));
}

void Engine::touchesMoved(const ds::ui::TouchEvent &e) {
	mTouchMovedEvents.incoming(mTouchTranslator.toWorldSpace(e));
}

void Engine::touchesEnded(const ds::ui::TouchEvent &e) {
	mTouchEndedEvents.incoming(mTouchTranslator.toWorldSpace(e));
}

ci::tuio::Client &Engine::getTuioClient() {
	return mTuio;
}

void Engine::mouseTouchBegin(const ci::app::MouseEvent &e, int id) {
	if (ds::ui::TouchMode::hasMouse(mTouchMode)) {
		mMouseBeginEvents.incoming(MousePair(alteredMouseEvent(e), id));
	}
}

void Engine::mouseTouchMoved(const ci::app::MouseEvent &e, int id) {
	if (ds::ui::TouchMode::hasMouse(mTouchMode)) {
		mMouseMovedEvents.incoming(MousePair(alteredMouseEvent(e), id));
	}
}

void Engine::mouseTouchEnded(const ci::app::MouseEvent &e, int id) {
	if (ds::ui::TouchMode::hasMouse(mTouchMode)) {
		mMouseEndedEvents.incoming(MousePair(alteredMouseEvent(e), id));
	}
}

ci::app::MouseEvent Engine::alteredMouseEvent(const ci::app::MouseEvent& e) const {
	// Note -- breaks the button and modifier checks, because cinder doesn't give me access to the raw data.
	// Currently I believe that's fine -- and since our target is touch platforms without those things
	// hopefully it always will be.
	// Note that you CAN get to this if you want to interpret what's there. I *think* I saw that
	// the newer version of cinder gave access so hopefully can just wait for that if we need it.

	// Transform the mouse event coordinate from window/screen space to world
	// coordinates.  Note: We are using the actual window size here, not the
	// mDstRect size, which may be different if the window gets automagically
	// resized for some reason.  (For example, in fullscreen mode, or a window
	// mode on a screen that is not big enough for the mDstRect)
	const ci::vec2 srcOffset = mData.mSrcRect.getUpperLeft();

	auto screenSize = glm::vec2( ci::app::getWindowSize() );
	const ci::vec2 screenScale = screenSize / mData.mSrcRect.getSize();

	const ci::vec2 mouseWorldPos = srcOffset + (ci::vec2(e.getX(), e.getY()) / screenScale);
	const ci::ivec2 pos((int)mouseWorldPos.x, (int)mouseWorldPos.y);

	return ci::app::MouseEvent(e.getWindow(),	0, pos.x, pos.y,
												0, e.getWheelIncrement(), e.getNativeModifiers());
}

void Engine::injectTouchesBegin(const ds::ui::TouchEvent& e){
	touchesBegin(e);
}

void Engine::injectTouchesMoved(const ds::ui::TouchEvent& e){
	touchesMoved(e);
}

void Engine::injectTouchesEnded(const ds::ui::TouchEvent& e){
	touchesEnded(e);
}

void Engine::injectObjectsBegin(const ds::TuioObject& o) {
	mTuioObjectsBegin.incoming(o);
}

void Engine::injectObjectsMoved(const ds::TuioObject& o) {
	mTuioObjectsMoved.incoming(o);
}

void Engine::injectObjectsEnded(const ds::TuioObject& o) {
	mTuioObjectsEnded.incoming(o);
}

ds::ResourceList& Engine::getResources() {
	return mResources;
}

const ds::FontList& Engine::getFonts() const {
	return mFonts;
}

ds::FontList& Engine::editFonts() {
	return mFonts;
}

const ds::ColorList& Engine::getColors() const {
	return mColors;
}

ds::ColorList& Engine::editColors() {
	return mColors;
}

void Engine::stopServices() {
	if (mData.mServices.empty()) return;

	for (auto it=mData.mServices.begin(), end=mData.mServices.end(); it!=end; ++it) {
		ds::EngineService*	s = it->second;
		if (s) s->stop();
	}
}

void Engine::setHideMouse(const bool doMouseHide){
	mHideMouse = doMouseHide;
}

bool Engine::getHideMouse() const {
	return mHideMouse;
}

ds::ui::Sprite* Engine::getHit(const ci::vec3& point) {
	for (auto it=mRoots.rbegin(), end=mRoots.rend(); it!=end; ++it) {
		ds::ui::Sprite* s = (*it)->getHit(point);
		if (s) return s;
	}
	return nullptr;
}

void Engine::clearFingers( const std::vector<int> &fingers ) {
	mTouchManager.clearFingers(fingers);
}

void Engine::nextTouchMode() {
	setTouchMode(ds::ui::TouchMode::next(mTouchMode));
}

void Engine::setTouchSmoothing(const bool doSmoothing){
	mTouchManager.setTouchSmoothing(doSmoothing);
}

void Engine::setTouchSmoothFrames(const int smoothFrames){
	mTouchManager.setTouchSmoothFrames(smoothFrames);
}

const bool Engine::getTouchSmoothing(){
	return mTouchManager.getTouchSmoothing();
}

void Engine::writeSprites(std::ostream &s) const {
#ifdef _DEBUG
	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		EngineRoot*		er(it->get());
		ds::ui::Sprite*	sprite(er ? er->getSprite() : nullptr);
		if (sprite) sprite->write(s, 0);
	}
#endif
}

bool Engine::isIdling() const {
	return mIdling;
}

void Engine::startIdling() {
	mIdling = true;
}

void Engine::resetIdleTimeout() {
	//DS_LOG_INFO("ResetIdleTimeout");
	float curr = static_cast<float>(ci::app::getElapsedSeconds());
	mLastTime = curr;
	mLastTouchTime = curr;
	mIdling = false;
}

void Engine::setTouchMode(const ds::ui::TouchMode::Enum &mode) {
	mTouchMode = mode;
	mTouchManager.setTouchMode(mode);
}

ci::app::WindowRef Engine::getWindow(){
	return mCinderWindow;
}

bool Engine::getRotateTouchesDefault(){
	return mRotateTouchesDefault;
}

/**
 * \class ds::Engine::Channel
 */
Engine::Channel::Channel() {
}

Engine::Channel::Channel(const std::string &description)
		: mDescription(description) {
}

} // namespace ds


namespace {

ds::EngineRoot*		find_master(const ds::RootList::Root::Type t, std::vector<std::unique_ptr<ds::EngineRoot>>& list) {
	for (auto it=list.begin(), end=list.end(); it!=end; ++it) {
		ds::EngineRoot*		r(it->get());
		if (!r) continue;
		if (r->getBuilder().mType == t && r->getBuilder().mMaster == ds::RootList::Root::kMaster) {
			return r;
		}
	}
	return nullptr;
}

void				root_setup(std::vector<std::unique_ptr<ds::EngineRoot>>& dst) {
	// Go through each of my roots, searching for a master. If I have a master, hook up all my slaves.
	for (auto it=dst.begin(), end=dst.end(); it!=end; ++it) {
		ds::EngineRoot*		r(it->get());
		if (!r) continue;
		if (r->getBuilder().mMaster == ds::RootList::Root::kSlave) {
			ds::EngineRoot*	master = find_master(r->getBuilder().mType, dst);
			if (master && master != r) {
				r->slaveTo(master);
			}
		}
	}
}

}
