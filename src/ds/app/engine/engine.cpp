#include "ds/app/engine/engine.h"
#include "ds/app/app.h"
#include "ds/app/auto_draw.h"
#include "ds/app/environment.h"
#include "ds/app/engine/engine_roots.h"
#include "ds/app/engine/engine_service.h"
#include "ds/app/engine/engine_stats_view.h"
#include "ds/app/error.h"
#include "ds/cfg/settings.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/math/math_defs.h"
#include "ds/ui/ip/ip_defs.h"
#include "ds/ui/ip/functions/ip_circle_mask.h"
#include "ds/ui/touch/draw_touch_view.h"
#include "ds/ui/touch/touch_event.h"

#include <cinder/Display.h>

//! This entire header is included for one single
//! function Poco::Path::expand. This slowly needs
//! to get removed. Poco is not part of the Cinder.
#include <Poco/Path.h>

#include <boost/algorithm/string/predicate.hpp>

#include "renderers/engine_renderer_null.h"
#include "renderers/engine_renderer_continuous_fxaa.h"
#include "renderers/engine_renderer_continuous.h"
#include "renderers/engine_renderer_discontinuous.h"

#pragma warning (disable : 4355)    // disable 'this': used in base member initializer list

namespace {
const int			NUMBER_OF_NETWORK_THREADS = 2;

void				root_setup(std::vector<std::unique_ptr<ds::EngineRoot>>&);

}

const ds::BitMask	ds::ENGINE_LOG = ds::Logger::newModule("engine");

namespace ds {

const int Engine::NumberOfNetworkThreads = 2;

Engine::Engine(	ds::App& app, const ds::cfg::Settings &settings,
				ds::EngineData& ed, const RootList& _roots)
	: ds::ui::SpriteEngine(ed)
	, mTweenline(app.timeline())
	, mIdling(true)
	, mTouchMode(ds::ui::TouchMode::kTuioAndMouse)
	, mTouchManager(*this, mTouchMode)
	, mSettings(settings)
	, mTouchBeginEvents(mTouchMutex,	mLastTouchTime, mIdling, [&app, this](const ds::ui::TouchEvent& e) {app.onTouchesBegan(e); this->mTouchManager.touchesBegin(e);})
	, mTouchMovedEvents(mTouchMutex,	mLastTouchTime, mIdling, [&app, this](const ds::ui::TouchEvent& e) {app.onTouchesMoved(e); this->mTouchManager.touchesMoved(e); })
	, mTouchEndEvents(mTouchMutex,		mLastTouchTime, mIdling, [&app, this](const ds::ui::TouchEvent& e) {app.onTouchesEnded(e); this->mTouchManager.touchesEnded(e); })
	, mMouseBeginEvents(mTouchMutex,	mLastTouchTime, mIdling, [this](const MousePair& e)  {handleMouseTouchBegin(e.first, e.second);})
	, mMouseMovedEvents(mTouchMutex,	mLastTouchTime, mIdling, [this](const MousePair& e)  {handleMouseTouchMoved(e.first, e.second);})
	, mMouseEndEvents(mTouchMutex,		mLastTouchTime, mIdling, [this](const MousePair& e)  {handleMouseTouchEnded(e.first, e.second);})
	, mTuioObjectsBegin(mTouchMutex,	mLastTouchTime, mIdling, [&app](const TuioObject& e) {app.tuioObjectBegan(e);})
	, mTuioObjectsMoved(mTouchMutex,	mLastTouchTime, mIdling, [&app](const TuioObject& e) {app.tuioObjectMoved(e);})
	, mTuioObjectsEnd(mTouchMutex,		mLastTouchTime, mIdling, [&app](const TuioObject& e) {app.tuioObjectEnded(e);})
	, mHideMouse(false)
	, mUniqueColor(0, 0, 0)
	, mAutoDraw(new AutoDrawService())
	, mCachedWindowW(0)
	, mCachedWindowH(0)
	, mAverageFps(0.0f)
{
	addChannel(ERROR_CHANNEL, "A master list of all errors in the system.");
	addService("ds/error", *(new ErrorService(*this)));

	// For now, install some default image processing functions here, for convenience. These are
	// so lightweight it probably makes sense just to have them always available for clients instead
	// of requiring some sort of configuration.
	mIpFunctions.add(ds::ui::ip::CIRCLE_MASK, ds::ui::ip::FunctionRef(new ds::ui::ip::CircleMask()));

	if (mAutoDraw) addService("AUTODRAW", *mAutoDraw);

	ds::Environment::loadSettings("debug.xml", mDebugSettings);
	ds::Logger::setup(mDebugSettings);

	// touch settings
	mTouchMode = ds::ui::TouchMode::fromSettings(settings);
	setTouchMode(mTouchMode);
	mTouchManager.setOverrideTranslation(settings.getBool("touch_overlay:override_translation", 0, false));
	mTouchManager.setOverrideDimensions(settings.getSize("touch_overlay:dimensions", 0, ci::Vec2f(1920.0f, 1080.0f)));
	mTouchManager.setOverrideOffset(settings.getSize("touch_overlay:offset", 0, ci::Vec2f(0.0f, 0.0f)));
	mTouchManager.setTouchFilterRect(settings.getRect("touch_overlay:filter_rect", 0, ci::Rectf(0.0f, 0.0f, 0.0f, 0.0f)));

	mData.mAppInstanceName = settings.getText("platform:guid", 0, "Downstream");

	const bool			drawTouches = settings.getBool("touch_overlay:debug", 0, false);
	mData.mMinTapDistance = settings.getFloat("tap_threshold", 0, 30.0f);
	mData.mMinTouchDistance = settings.getFloat("touch:minimum_distance", 0, 10.0f);
	mData.mSwipeQueueSize = settings.getInt("touch:swipe:queue_size", 0, 4);
	mData.mSwipeMinVelocity = settings.getFloat("touch:swipe:minimum_velocity", 0, 800.0f);
	mData.mSwipeMaxTime = settings.getFloat("touch:swipe:maximum_time", 0, 0.5f);
	mData.mFrameRate = settings.getFloat("frame_rate", 0, 60.0f);
	mFxaaOptions.mApplyFxAA = settings.getBool("FxAA", 0, false);
	mFxaaOptions.mFxAASpanMax = settings.getFloat("FxAA:SpanMax", 0, 2.0);
	mFxaaOptions.mFxAAReduceMul = settings.getFloat("FxAA:ReduceMul", 0, 8.0);
	mFxaaOptions.mFxAAReduceMin = settings.getFloat("FxAA:ReduceMin", 0, 128.0);

	mData.mWorldSize = settings.getSize("world_dimensions", 0, ci::Vec2f(640.0f, 400.0f));
	// Backwards compatibility with pre src-dst rect days
	const float				DEFAULT_WINDOW_SCALE = 1.0f;
	if (settings.getRectSize("local_rect") > 0) {
		const float			window_scale = mDebugSettings.getFloat("window_scale", 0, DEFAULT_WINDOW_SCALE);
		mData.mSrcRect = settings.getRect("local_rect", 0, mData.mSrcRect);
		mData.mDstRect = ci::Rectf(0.0f, 0.0f, mData.mSrcRect.getWidth(), mData.mSrcRect.getHeight());
		if (settings.getPointSize("window_pos") > 0) {
			const ci::Vec3f	size(settings.getPoint("window_pos"));
			mData.mDstRect.offset(size.xy());
		}
	}
	// Src rect and dst rect are new, and should obsolete local_rect. For now, default to illegal values,
	// which makes them get ignored.
	if (settings.getRectSize("src_rect") > 0 || settings.getRectSize("dst_rect") > 0) {

	    //! Size of dst_rect must match src_rect. This is a must! otherwise how stuff should be rendered?
		DS_ASSERT_MSG(settings.getRectSize("src_rect") == settings.getRectSize("dst_rect"), "src_rect num must match dst_rect num.");

		const ci::Rectf		empty_rect(0.0f, 0.0f, -1.0f, -1.0f);

		//! If we are asked to render discontinuous parts of the world, special care is required.
		// here's the procedure:
		// in case I found more than on dst_rect / src_rect in engine.xml, I know I have been asked
		// to render discontinued parts of the world. Therefore, what I will do is that I will first
		// render the entire world into one single FBO and then take little "chunks" of its texture (
		// specified by src_rect's) and will draw at specific locations (marked by dst_rect's). In this
		// setup however, I will need good'ol screen_rect config entry back to figure out the window
		// position / size because src_rect's and dst_rect's do not specify anything about the window
		// anymore.

		if (settings.getRectSize("src_rect") > 1)
		{
			mData.mScreenRect = ci::Rectf::zero();
			auto expand_screen_rect_by_dst_rect = [this](const ci::Rectf& dst) { 
				static std::once_flag initial_flag;
				std::call_once(initial_flag, [this, dst]{ mData.mScreenRect = dst; });

				if (dst.getUpperLeft().x < mData.mScreenRect.getUpperLeft().x)
				{
					mData.mScreenRect.x1 = dst.getUpperLeft().x;
				}

				if (dst.getUpperLeft().y < mData.mScreenRect.getUpperLeft().y)
				{
					mData.mScreenRect.y1 = dst.getUpperLeft().y;
				}

				if (dst.getLowerRight().x > mData.mScreenRect.getLowerRight().x)
				{
					mData.mScreenRect.x2 = dst.getLowerRight().x;
				}

				if (dst.getLowerRight().y > mData.mScreenRect.getLowerRight().y)
				{
					mData.mScreenRect.y2 = dst.getLowerRight().y;
				}
			};
			
			// mData.mSrcRect and mData.mDstRect equal to world will force the entire world
			// to be rendered into a single FBO.
			mData.mSrcRect = ci::Rectf(ci::Vec2f::zero(), mData.mWorldSize);
			mData.mDstRect = ci::Rectf(ci::Vec2f::zero(), mData.mWorldSize);

			// Get all "chunks" of the world we've been asked to render.
			for (auto index = 0, end = settings.getRectSize("src_rect"); index < end; ++index)
			{
				const auto src_rect = settings.getRect("src_rect", index, empty_rect);
				const auto dst_rect = settings.getRect("dst_rect", index, empty_rect);

				expand_screen_rect_by_dst_rect(dst_rect);

				mData.mWorldSlices.push_back(std::make_pair(
					ci::Area(	static_cast<int>(src_rect.getUpperLeft().x),
								static_cast<int>(src_rect.getUpperLeft().y),
								static_cast<int>(src_rect.getLowerRight().x),
								static_cast<int>(src_rect.getLowerRight().y))
					, // FBOs are flipped, therefore subtract world height from my Y pos
					ci::Rectf(dst_rect.x1, mData.mWorldSize.y - dst_rect.y1, dst_rect.x2, mData.mWorldSize.y - dst_rect.y2)
					));
			}
		}
		else
		{
			// normal continuous-render engine behavior.
			mData.mSrcRect = settings.getRect("src_rect", 0, empty_rect);
			mData.mDstRect = settings.getRect("dst_rect", 0, empty_rect);
		}
	}
	// Override the screen rect if we're using the new-style mode. I inherit behaviour like setting
	// the window size from this.
	if (mData.mDstRect.x2 > mData.mDstRect.x1 && mData.mDstRect.y2 > mData.mDstRect.y1) {
		// Hmmm... suspect the screen rect does not support setting x1, y1, because when I do
		// everything goes black. That really needs to be weeded out in favour of the new system.
		mData.mScreenRect = ci::Rectf(0.0f, 0.0f, mData.mDstRect.getWidth(), mData.mDstRect.getHeight());
	}

	if(mData.mScreenRect.getWidth() < 1 || mData.mScreenRect.getHeight() < 1){
		DS_LOG_WARNING("Screen rect is 0 width or height. Overriding to full screen size");
		ci::DisplayRef mainDisplay = ci::Display::getMainDisplay();
		ci::Rectf mainDisplayRect = ci::Rectf(0.0f, 0.0f, (float)mainDisplay->getWidth(), (float)mainDisplay->getHeight());
		mData.mSrcRect = mainDisplayRect;
		mData.mDstRect = mainDisplayRect;
		mData.mScreenRect = mainDisplayRect;
		mData.mWorldSize = ci::Vec2f(mainDisplayRect.getWidth(), mainDisplayRect.getHeight());
	}


	// Don't construct roots on startup for clients, and instead create them when we connect to a server
	const std::string	arch(settings.getText("platform:architecture", 0, ""));
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
			Picking*						picking = nullptr;
			if(r.mPick == r.kSelect) picking = &mSelectPicking;
			std::unique_ptr<EngineRoot>		root;
			if(r.mType == r.kOrtho) root.reset(new OrthRoot(*this, r, r.mRootId));
			else if(r.mType == r.kPerspective) root.reset(new PerspRoot(*this, r, r.mRootId, r.mPersp, picking));
			if(!root) throw std::runtime_error("Engine can't create root");
			mRoots.push_back(std::move(root));
			--root_id;
		}
		if(mRoots.empty()) {
			throw std::runtime_error("Engine can't create single root");
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
	const EngineRoot::Settings	er_settings(mData.mWorldSize, mData.mScreenRect, mDebugSettings, DEFAULT_WINDOW_SCALE, mData.mSrcRect, mData.mDstRect);
	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		EngineRoot&				r(*(it->get()));
		r.setup(er_settings);
	}

	mRotateTouchesDefault = settings.getBool("touch:rotate_touches_default", 0, false);

	// SETUP PICKING
	mSelectPicking.setWorldSize(mData.mWorldSize);

	// SETUP RESOURCES
	std::string resourceLocation = settings.getText("resource_location", 0, "");
	if (resourceLocation.empty()) {
		// This is valid, though unusual
		std::cout << "Engine() has no resource_location setting" << std::endl;
	} else {
		resourceLocation = Poco::Path::expand(resourceLocation);
		Resource::Id::setupPaths(resourceLocation, settings.getText("resource_db", 0), settings.getText("project_path", 0));
	}

	setIdleTimeout(settings.getInt("idle_time", 0, 300));

	std::cout << "Engine constructor complete, app instance name: " << mData.mAppInstanceName << std::endl;
}


void Engine::prepareSettings(ci::app::AppBasic::Settings& settings){
	// TODO: remove this null_renderer bullshit
	std::string screenMode = "window";
	if(mSettings.getBoolSize("null_renderer") > 0 && mSettings.getBool("null_renderer"))
	{
		// a 50x25 window for null renderer.
		settings.setWindowSize(50, 25);
		settings.setFullScreen(false);
		settings.setBorderless(false);
		settings.setAlwaysOnTop(false);

	} else {
		settings.setWindowSize(static_cast<int>(getWidth()), static_cast<int>(getHeight()));
		if(mData.mUsingDefaults){
			screenMode = "borderless";
		}
		screenMode = mSettings.getText("screen:mode", 0, screenMode);
		if(screenMode == "full"){
			settings.setFullScreen(true);
		} else if(screenMode == "fullscreen"){
			settings.setFullScreen(true);
		} else if(screenMode == "borderless"){
			settings.setBorderless(true);
		}

		settings.setAlwaysOnTop(mSettings.getBool("screen:always_on_top", 0, false));
	}
	DS_LOG_INFO("Engine::prepareSettings: screenMode is " << screenMode << " and always on top " << settings.isAlwaysOnTop());

	settings.setResizable(false);

	if(ds::ui::TouchMode::hasSystem(mTouchMode)) {
		settings.enableMultiTouch();
	}

	mHideMouse = mSettings.getBool("hide_mouse", 0, mHideMouse);
	mTuioPort = mSettings.getInt("tuio_port", 0, 3333);
	setTouchSmoothing(mSettings.getBool("touch_smoothing", 0, true));
	setTouchSmoothFrames(mSettings.getInt("touch_smooth_frames", 0, 5));

	settings.setFrameRate(mData.mFrameRate);

	const std::string     nope = "ds:IllegalTitle";
	const std::string     title = mSettings.getText("screen:title", 0, nope);
	if(title != nope) settings.setTitle(title);
}

void Engine::setup(ds::App& app) {

	mCinderWindow = app.getWindow();

	mTouchTranslator.setTranslation(mData.mSrcRect.x1, mData.mSrcRect.y1);
	mTouchTranslator.setScale(mData.mSrcRect.getWidth() / ci::app::getWindowWidth(), mData.mSrcRect.getHeight() / ci::app::getWindowHeight());


	const std::string	arch(mSettings.getText("platform:architecture", 0, ""));
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
		DS_LOG_FATAL("Engine::setup() on 0 size width or height");
		std::cout << "ERROR Engine::setup() on 0 size width or height" << std::endl;
		throw std::runtime_error("Engine::setup() on 0 size width or height");
	}
	//////////////////////////////////////////////////////////////////////////

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

	setupRenderer();
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
		if(!root) throw std::runtime_error("Engine can't create root");
		mRoots.push_back(std::move(root));
		if(thisRootId < root_id){
			root_id = thisRootId;
		}
	}

	--root_id;
	createStatsView(root_id);
	root_setup(mRoots);

	const EngineRoot::Settings	er_settings(mData.mWorldSize, mData.mScreenRect, mDebugSettings, 1.0f, mData.mSrcRect, mData.mDstRect);
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
				mRoots.push_back(std::move(root));
			}
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
	if (name.empty()) throw std::runtime_error("Engine::getChannel() on empty name");
	if (!mChannels.empty()) {
		auto f = mChannels.find(name);
		if (f != mChannels.end()) return f->second.mNotifier;
	}
	mChannels[name] = Channel();
	auto f = mChannels.find(name);
	if (f != mChannels.end()) return f->second.mNotifier;
	throw std::runtime_error("Engine::getChannel() no channel named " + name);
}

void Engine::addChannel(const std::string &name, const std::string &description) {
	if (name.empty()) throw std::runtime_error("Engine::addChannel() on empty name");
	mChannels[name] = Channel(description);
}

ds::AutoUpdateList& Engine::getAutoUpdateList(const int mask) {
	if ((mask&AutoUpdateType::SERVER) != 0) return mAutoUpdateServer;
	if ((mask&AutoUpdateType::CLIENT) != 0) return mAutoUpdateClient;
	throw std::runtime_error("Engine::getAutoUpdateList() on illegal param");
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
	mData.mEngineCfg.loadText(filename, this);
}

void Engine::loadNinePatchCfg(const std::string& filename) {
	mData.mEngineCfg.loadNinePatchCfg(filename);
}

int Engine::getRootCount() const {
	return mRoots.size();
}

ui::Sprite& Engine::getRootSprite(const size_t index) {
	if (index < 0 || index >= mRoots.size()) throw std::runtime_error("Engine::getRootSprite() on invalid index");
	ui::Sprite*		s = mRoots[index]->getSprite();
	if (!s) throw std::runtime_error("Engine::getRootSprite() on null sprite");
	return *s;
}

const RootList::Root& Engine::getRootBuilder(const size_t index){
	if(index < 0 || index >= mRoots.size()) throw std::runtime_error("Engine::getRootBuilder() on invalid index");
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
		mMouseEndEvents.lockedUpdate();
	}

	mMouseBeginEvents.update(curr);
	mMouseMovedEvents.update(curr);
	mMouseEndEvents.update(curr);

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
		mMouseEndEvents.lockedUpdate();

		mTouchBeginEvents.lockedUpdate();
		mTouchMovedEvents.lockedUpdate();
		mTouchEndEvents.lockedUpdate();

		mTuioObjectsBegin.lockedUpdate();
		mTuioObjectsMoved.lockedUpdate();
		mTuioObjectsEnd.lockedUpdate();
	} // unlock touch mutex
	//////////////////////////////////////////////////////////////////////////

	mMouseBeginEvents.update(curr);
	mMouseMovedEvents.update(curr);
	mMouseEndEvents.update(curr);

	mTouchBeginEvents.update(curr);
	mTouchMovedEvents.update(curr);
	mTouchEndEvents.update(curr);

	mTuioObjectsBegin.update(curr);
	mTuioObjectsMoved.update(curr);
	mTuioObjectsEnd.update(curr);

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
	throw std::runtime_error("getPerspectiveCamera() on non-perspective root.");
}

const ci::CameraPersp& Engine::getPerspectiveCameraRef(const size_t index) const {
	const PerspRoot*			root = nullptr;
	if (index < mRoots.size()) root = dynamic_cast<const PerspRoot*>(mRoots[index].get());
	if (root) {
		return root->getCameraRef();
	}
	DS_LOG_ERROR(" Engine::getPerspectiveCamera() on invalid root (" << index << ")");
	throw std::runtime_error("getPerspectiveCamera() on non-perspective root.");
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
	throw std::runtime_error("getOrthoFarPlane() on non-perspective root.");
}

float Engine::getOrthoNearPlane(const size_t index)const{
	const OrthRoot*				root = nullptr;
	if(index < mRoots.size()) root = dynamic_cast<const OrthRoot*>(mRoots[index].get());
	if(root) {
		return root->getNearPlane();
	}
	DS_LOG_ERROR(" Engine::getOrthoNearPlane() on invalid root (" << index << ")");
	throw std::runtime_error("getOrthoNearPlane() on non-perspective root.");
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
	throw std::runtime_error("setOrthoViewPlanes() on non-perspective root.");
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
		client.registerObjectUpdated([this](ci::tuio::Object o) { this->mTuioObjectsMoved.incoming(TuioObject(o.getFiducialId(), o.getPos(), o.getAngle())); });
		client.registerObjectRemoved([this](ci::tuio::Object o) { this->mTuioObjectsEnd.incoming(TuioObject(o.getFiducialId(), o.getPos(), o.getAngle())); });
	}
}

void Engine::drawClient() {
	mRenderer->drawClient();
}

void Engine::drawServer() {
	mRenderer->drawServer();
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
	mTouchEndEvents.incoming(mTouchTranslator.toWorldSpace(e));
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
		mMouseEndEvents.incoming(MousePair(alteredMouseEvent(e), id));
	}
}

ci::app::MouseEvent Engine::alteredMouseEvent(const ci::app::MouseEvent& e) const {
	// Note -- breaks the button and modifier checks, because cinder doesn't give me access to the raw data.
	// Currently I believe that's fine -- and since our target is touch platforms without those things
	// hopefully it always will be.
	// Note that you CAN get to this if you want to interpret what's there. I *think* I saw that
	// the newer version of cinder gave access so hopefully can just wait for that if we need it.

	// Translate the mouse from the actual window to the desired rect in world coordinates.
	const ci::Vec2i	pos(mTouchTranslator.toWorldi(e.getX(), e.getY()));
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

bool Engine::hideMouse() const {
	return mHideMouse;
}

ds::ui::Sprite* Engine::getHit(const ci::Vec3f& point) {
	for (auto it=mRoots.rbegin(), end=mRoots.rend(); it!=end; ++it) {
		
		ci::Vec3f pointToUse = point;
		if((*it)->getSprite()->getPerspective()){
			// scale the point from world size to screen size (which is the size of the perspective root)
			pointToUse.set(
				(point.x / mData.mWorldSize.x) * mData.mScreenRect.getWidth(),
				(point.y / mData.mWorldSize.y) * mData.mScreenRect.getHeight(),
				0.0f
			);
		}
		ds::ui::Sprite* s = (*it)->getHit(pointToUse);
		if (s) return s;
	}
	return nullptr;
}

void Engine::clearFingers( const std::vector<int> &fingers ) {
	mTouchManager.clearFingers(fingers);
}

const ci::Rectf& Engine::getScreenRect() const {
	return mData.mScreenRect;
}

void Engine::translateTouchPoint(ci::Vec2f& inOutPoint) {
	inOutPoint = mTouchTranslator.toWorldf(inOutPoint.x, inOutPoint.y);
	if(mTouchManager.getOverrideEnabled()){
		mTouchManager.overrideTouchTranslation(inOutPoint);
	}
};

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
	if (mIdling)
		return;

	mIdling = true;
}

void Engine::resetIdleTimeout() {
	float curr = static_cast<float>(ci::app::getElapsedSeconds());
	mLastTime = curr;
	mLastTouchTime = curr;
	mIdling = false;
}

namespace {
ci::app::MouseEvent offset_mouse_event(const ci::app::MouseEvent& e, const ci::Rectf& offset) {
	// Note -- breaks the button and modifier checks, because cinder doesn't give me access to the raw data.
	// Currently I believe that's fine -- and since our target is touch platforms without those things
	// hopefully it always will be.
	return ci::app::MouseEvent(	e.getWindow(), 0, e.getX() + static_cast<int>(offset.x1), e.getY() + static_cast<int>(offset.y1),
								0, e.getWheelIncrement(), e.getNativeModifiers());
}

}

void Engine::setToUserCamera() {
	for (auto it=mRoots.begin(), end=mRoots.end(); it!=end; ++it) {
		(*it)->setViewport(false);
	}

	// When using a user camera, offset the event inputs.
	mMouseBeginEvents.setUpdateFn([this](const MousePair& e)  {this->handleMouseTouchBegin(offset_mouse_event(e.first, mData.mScreenRect), e.second); });
	mMouseMovedEvents.setUpdateFn([this](const MousePair& e)  {this->handleMouseTouchMoved(offset_mouse_event(e.first, mData.mScreenRect), e.second); });
	mMouseEndEvents.setUpdateFn([this](const MousePair& e)  {this->handleMouseTouchEnded(offset_mouse_event(e.first, mData.mScreenRect), e.second);});
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

void Engine::setupRenderer()
{
	//! multiple calls to this method should do nothing
	//! but also should not crash the whole thing!
	static std::once_flag renderer_initialized;
	
	//! decide and pick the most appropriate renderer and set it up.
	//! based on engine.xml entries.
	std::call_once(renderer_initialized, [this] {
		if (mSettings.getBoolSize("null_renderer") > 0 && mSettings.getBool("null_renderer"))
		{
			mRenderer = std::make_unique<EngineRendererNull>(*this);
		}
		else if (mData.mWorldSlices.empty()) //if continuous
		{
			if (mFxaaOptions.mApplyFxAA) //if with FXAA
			{
				mRenderer = std::make_unique<EngineRendererContinuousFxaa>(*this);
			}
			else //if no FXAA
			{
				mRenderer = std::make_unique<EngineRendererContinuous>(*this);
			}
		}
		else //if discontinuous
		{
			if (mFxaaOptions.mApplyFxAA) //if with FXAA
			{
				//! I can't figure out a way to mix these two yet. Pending! (TODO: SL)
				throw std::logic_error("FXAA is not supported in discontinuous rendering mode.");
			}
			else //if no FXAA
			{
				mRenderer = std::make_unique<EngineRendererDiscontinuous>(*this);
			}
		}
	});
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