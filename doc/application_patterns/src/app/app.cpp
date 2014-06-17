#include "na/app/app.h"

#include <ofxWorldEngineImageSprite.h>
#include <framework/debug/debug_ds.h>
#include <framework/util/resources_writer.h>
#include <ofxSupportDS/LoggerDs.h>
#include <ofxSupportDS/StringUtils.h>

#include "na/app/user_debug_layer.h"
#include "na/app/physics_world.h"
#include "na/events/app_events.h"
#include "na/query/rtn_query.h"
#include "na/ui/attract/attract_layer.h"
#include "na/ui/background/background_layer.h"
#include "na/ui/home/home_layer.h"
#include "na/ui/story/story_panel_layer.h"

using namespace na;

namespace {
const char*				up = "Can't create application";
const ds::BitMask APP_LOG   = ds::Logger::newModule("app");

static void add_sprite(WindowSprite& parent, WindowSprite* child, const char* error)
{
  if (!child) throw std::runtime_error(error);
  parent.addSprite(child);
}

/**
 * IdleWindow
 * Do nothing but wait to instigate the idle state.
 */
class IdleWindow : public WindowSprite
{
public:
  static IdleWindow& newSprite(na::Globals& g) {
    IdleWindow* ans = new IdleWindow(g);
    if (!ans) throw std::runtime_error("Can't create idle window");
    return *ans;
  }

  IdleWindow(na::Globals& g)
    : mMode(g.mMode)
    , mEventClient(g, [this](const na::Event* m){ if (m) this->onAppEvent(*m); })
  {
    setSecondsBeforeIdle(g.mLayout.getFloat("attract:time_to_start"));
  }

  virtual void        update() {
    WindowSprite::update();

    // Never idle out of presenter mode
    if (mMode.getMode() == WallMode::PRESENTER_MODE) return;

    if (isIdling()) {
      mMode.setToAttractMode();
    } else {
      // If my mode is idling, go to interactive, otherwise leave it be
      if (mMode.getMode() == WallMode::ATTRACT_MODE) mMode.setToHomeMode();
    }
  }

private:
  void                onAppEvent(const na::Event& e)
  {
    if (e.mWhat == na::WallModeChanged::WHAT()) {
      onWallModeChanged((const na::WallModeChanged&)e);
    }
  }

  void                onWallModeChanged(const na::WallModeChanged& m) {
    if (m.mMode == na::WallMode::ATTRACT_MODE) {
      // Whenever someone explicitly sets me to idle mode, make sure my window is actually
      // idling, or else I'll immediately go into interactive mode.
      startIdling();
    }
  }

  na::WallMode&       mMode;
  na::EventClient     mEventClient;
};


/**
 * LogoSprite
 * Display the logo (which needs to go away in presenter mode).
 */
class LogoSprite : public ImageSprite
{
public:
  LogoSprite(na::Globals& g)
    : ImageSprite(g.mLayout.getResourceId("netapp_logo"))
    , mEventClient(g, [this](const na::Event* m){ if (m) this->onAppEvent(*m); })
  {
    const ofxVec2f& logo_pos = g.mLayout.getSize("logo:position");

    setScale(g.mLayout.getFloat("logo:scale"));
    setPosition(g.mWorldFrame.width()*logo_pos.x, g.mWorldFrame.height()*logo_pos.y);

    mAnimFade.setEasing(kTweenEaseInOut);
  }

private:
  void                  onAppEvent(const na::Event& e)
  {
    if (e.mWhat == na::WallModeChanged::WHAT()) {
      onWallModeChanged((const na::WallModeChanged&)e);
    }
  }

  void                  onWallModeChanged(const na::WallModeChanged& m) {
    if (m.mMode == na::WallMode::PRESENTER_MODE) {
      mAnimFade.animateOff(*this, 0.5f);
    } else {
      mAnimFade.animateOn(*this, 0.5f);
    }
  }

  na::EventClient       mEventClient;
  na::anim::SimpleFade  mAnimFade;
};

}

/**
 * na::App
 */
App::App(const ds::AppSettings& appSettings)
	: mMouseHandler(mWorld)
	, mAppSettings(appSettings)
	, mGlobals(*this, mWorld)
  , mIdler(IdleWindow::newSprite(mGlobals))
  , mStoryTreeQuery(mWorld)
  , mStoryPanelRequestQuery(mWorld)
  , mStoryPageRequestQuery(mWorld)
//  , mOpenRtnEventQuery(mWorld)
  , mVirtueQuery(mWorld)
  , mPreloader(mWorld)
  , mDebug(mWorld, appSettings)
{
  mStoryTreeQuery.setReplyHandler([this](StoryTreeQuery& q){this->onStoryTreeResult(q);});
  mStoryPanelRequestQuery.setReplyHandler([this](na::AnnotatedStoryQuery<na::StoryPanelRequestQuery>& q){this->onStoryPanelRequestQuery(q);});
  mStoryPageRequestQuery.setReplyHandler([this](na::AnnotatedStoryQuery<na::StoryPageRequestQuery>& q){this->onStoryPageRequestQuery(q);});
//  mOpenRtnEventQuery.setReplyHandler([this](na::AnnotatedRtnQuery<na::OpenRtnEvent>& q){this->onOpenRtnEventQuery(q);});
  mVirtueQuery.setReplyHandler([this](na::VirtueQuery& q){q.composite(this->mVirtue);});
  mGlobals.mEventNotifier.addListener(this, [this](const na::Event* m){ if (m) this->onAppEvent(*m); });
}

void App::setup()
{
  DS_LOG_INFO_M("NetApp Data Wall setup()", na::APP_LOG);
	mWorld.start(60, mAppSettings);
	mWorld.setUpdateFunction(this, &App::worldUpdate);

	// Give any local settings a chance to load
	mGlobals.initialize(mAppSettings.getRawProjectPath());
	DS_DBG_CODE(if (mGlobals.mDebug.getBool("enable_resource_writer", 0, false)) {
		ds::ResourcesWriter	writer("na", "na/app/app_resources", "../src/"); writer.run();
	});
  mDebug.setup(mGlobals);

  mPreloader.add(mGlobals.mLayout.getResourceId("story:panel:bg"));
  mVirtueQuery.start();
  // The RTNs aren't live right now so we can load once
  RtnQuery        rtnQ(mGlobals.mRtnResources, false);
  rtnQ.setInputForAll();
  rtnQ.run();
  mRtnList.swap(rtnQ.mOutput);

  // This got set before local settings were loaded.
  mIdler.setSecondsBeforeIdle(mGlobals.mLayout.getFloat("attract:time_to_start"));

  // Make things a little easier on ourselves by always having the root data available
  // when we need it.  If I don't do this, then the idle cycle has to  be written to delay
  // construction until the data arrives, which is a waste of time, since the top level
  // won't ever change.
  // Plus, other parts of the API rely on this setup, so set it in stone.
  {
    StoryTreeQuery      q;
    q.run();
    if (q.mError || q.mOutput.empty()) {
      DS_LOG_FATAL_M("No story tree information", APP_LOG);
      throw std::runtime_error("No story tree information");
    }
    mStoryTree.swap(q.mOutput);
    std::vector<StoryLink>  sln = mStoryTree.getRoots(StoryLink::Solutions);
    std::vector<StoryLink>  ppl = mStoryTree.getRoots(StoryLink::People);
    if (sln.size() != 4 || ppl.size() != 4) {
      DS_LOG_FATAL_M("Tree story does not contain the needed roots (4 solution, 4 people)", APP_LOG);
      throw std::runtime_error("Tree story does not contain the needed roots (4 solution, 4 people)");
    }
    // Fill the macros
    const std::string       epic = mGlobals.mLayout.getText("story:epic:breadcrumb", 0, "");
    const na::StoryLink*    link = mStoryTree.findByBreadcrumb(epic);
    if (link) {
      mGlobals.mCustomerEpicStories = link->getId();
    } else {
      DS_LOG_ERROR_M("Tree story missing breadcrumb (" << epic << ")", APP_LOG);
    }
  }
//  mStoryTreeQuery.start(nullptr);

	DS_DBG_CODE(mWorld.getMasterWindow()->setScale(mGlobals.mDebug.getFloat("world_scale", 0, 1.0f)));

  // Make sure to construct the physics before anyone starts up
  mGlobals.mPhysicsWorld = &(PhysicsWorld::createOn(*(mWorld.getMasterWindow()), mGlobals));

  mWorld.addSprite(&mIdler);

  // background
  add_sprite(mIdler, new ui::BackgroundLayer(mGlobals), "Can't create background layer");
  // attract
  add_sprite(mIdler, new attract::Layer(mGlobals), "Can't create attract layer");
  // home
  add_sprite(mIdler, new ui::HomeLayer(mGlobals), "Can't create home layer");
  // Story panels
  add_sprite(mIdler, new ui::StoryPanelLayer(mGlobals), "Can't create story panel layer");
  // logo, always on top
  ImageSprite*      logo = new LogoSprite(mGlobals);
  if (logo) mWorld.addSprite(logo);

  // Special runtime debug mode
  if (mGlobals.mDebug.getBool("user_debug_layer", 0, false)) {
    WindowSprite*   ud = new UserDebugLayer(mGlobals);
    if (ud) mWorld.addSprite(ud);
  }

  mGlobals.mMode.setToAttractMode();
}

void App::exit()
{
  // Ugh... the app is partially cleaned up at this point, can't do this
//  mGlobals.notify(na::AppExitEvent());
}

void App::draw()
{
	mWorld.drawSimulator();
}

void App::worldUpdate()
{
  mPreloader.update();
  mDebug.update();
}

void App::keyPressed(int key)
{
  mDebug.keyPressed(key);
}

void App::keyReleased(int key)
{
  mDebug.keyReleased(key);
}

void App::onAppEvent(const na::Event& e)
{
  if (e.mWhat == na::StoryPanelRequestQuery::WHAT()) {
    // First we need to get the story info
    const na::StoryPanelRequestQuery&   r((const na::StoryPanelRequestQuery&)e);
    mStoryPanelRequestQuery.start([&r](na::AnnotatedStoryQuery<na::StoryPanelRequestQuery>& q){q.setInputFromRowId(r.mStoryId); q.mT = r;});
  } else if (e.mWhat == na::StoryPageRequestQuery::WHAT()) {
    // First we need to get the story info
    const na::StoryPageRequestQuery&   r((const na::StoryPageRequestQuery&)e);
    mStoryPageRequestQuery.start([&r](na::AnnotatedStoryQuery<na::StoryPageRequestQuery>& q){q.setInputFromRowId(r.mStoryId); q.mT = r;});
  } else if (e.mWhat == na::OpenRtnEvent::WHAT()) {
    const na::OpenRtnEvent&   r((const na::OpenRtnEvent&)e);
//    mOpenRtnEventQuery.start([&r](na::AnnotatedRtnQuery<na::OpenRtnEvent>& q){q.setInputFromRowId(r.mRowId); q.mT = r;});
    onOpenRtnEventQuery(r);
  }
}

void App::onStoryTreeResult(StoryTreeQuery& q)
{
  if (!q.mError) {
    mStoryTree.swap(q.mOutput);
  }
}

void App::onStoryPanelRequestQuery(na::AnnotatedStoryQuery<na::StoryPanelRequestQuery>& q)
{
  if (q.mOutput.empty()) {
    DS_LOG_WARNING_M("App::onStoryPanelRequestQuery() no story for ID=" << q.mT.mStoryId, na::APP_LOG);
    return;
  }
  StoryPanelRequest   spr;
  spr.mStory.swap(q.mOutput[0]);
  spr.mGlobalPosition = q.mT.mGlobalPosition;
  mGlobals.mEventNotifier.notify(&spr);
}

void App::onStoryPageRequestQuery(na::AnnotatedStoryQuery<na::StoryPageRequestQuery>& q)
{
  if (q.mOutput.empty()) {
    DS_LOG_WARNING_M("App::onStoryPageRequestQuery() no story for ID=" << q.mT.mStoryId, na::APP_LOG);
    return;
  }
  StoryPageRequest   spr(q.mOutput[0]);
  spr.mRoute = q.mT.mRoute;
  spr.mPageNumber = q.mT.mPageNumber;
  spr.mReplaceContent = q.mT.mReplaceContent;
  mGlobals.mEventNotifier.notify(&spr);
}

#if 0
void App::onOpenRtnEventQuery(na::AnnotatedRtnQuery<na::OpenRtnEvent>& q)
{
  if (q.mOutput.empty()) {
    DS_LOG_WARNING_M("App::onOpenRtnEventQuery() no story for ID=" << q.mT.mRowId, na::APP_LOG);
    return;
  }
  na::OpenRtnReply           reply(q.mT.mRoute);
  reply.mRtn.swap(q.mOutput[0]);
  mGlobals.mEventNotifier.notify(&reply);
  // Put it back, so the memory can be reused
  reply.mRtn.swap(q.mOutput[0]);
}
#endif

void App::onOpenRtnEventQuery(const na::OpenRtnEvent& re)
{
  const Rtn*        rtn = mRtnList.find(re.mRowId);
  if (!rtn) {
    DS_LOG_WARNING_M("App::onOpenRtnEventQuery() no story for ID=" << re.mRowId, na::APP_LOG);
    return;
  }
  na::OpenRtnReply           reply(re.mRoute, *rtn);
  mGlobals.mEventNotifier.notify(&reply);
}
