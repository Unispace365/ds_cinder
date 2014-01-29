#include "ds/app/app.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include "ds/app/engine/engine_client.h"
#include "ds/app/engine/engine_clientserver.h"
#include "ds/app/engine/engine_server.h"
#include "ds/debug/console.h"
#include "ds/debug/logger.h"
#include "ds/debug/debug_defines.h"
// For installing the sprite types
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/nine_patch.h"
#include "ds/ui/sprite/text.h"
// For installing the image generators
#include "ds/ui/image_source/image_arc.h"
#include "ds/ui/image_source/image_file.h"
#include "ds/ui/image_source/image_resource.h"
// For verifying that the resources are installed
#include "ds/app/FrameworkResources.h"

// Answer a new engine based on the current settings
static ds::Engine&    new_engine(ds::App&, const ds::cfg::Settings&, ds::EngineData&, const std::vector<int>* roots);

static std::vector<std::function<void(ds::Engine&)>>& get_startups()
{
	static std::vector<std::function<void(ds::Engine&)>>	VEC;
	return VEC;
}

namespace {
std::string				APP_PATH;
std::string				APP_DATA_PATH;
#ifdef _DEBUG
ds::Console				GLOBAL_CONSOLE;
#endif
}

namespace ds {

void App::AddStartup(const std::function<void(ds::Engine&)>& fn)
{
	if (fn != nullptr) get_startups().push_back(fn);
}

/**
 * \class ds::App
 */
App::App(const std::vector<int>* roots)
	: mInitializer(getAppPath().generic_string())
	, mEngineSettings()
	, mEngine(new_engine(*this, mEngineSettings, mEngineData, roots))
	, mCtrlDown(false)
	, mSecondMouseDown(false)
	, mQKeyEnabled(false)
	, mEscKeyEnabled(false)
	, mArrowKeyCameraStep(mEngineSettings.getFloat("camera:arrow_keys", 0, -1.0f))
	, mArrowKeyCameraControl(mArrowKeyCameraStep > 0.025f)
{
	// Initialize each sprite type with a unique blob handler for network communication.
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Sprite::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Sprite::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Image::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Image::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::NinePatch::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::NinePatch::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Text::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Text::installAsClient(r);});

	// Initialize the engine image generator typess.
	ds::ui::ImageArc::install(mEngine.getImageRegistry());
	ds::ui::ImageFile::install(mEngine.getImageRegistry());
	ds::ui::ImageResource::install(mEngine.getImageRegistry());

	if (mArrowKeyCameraControl) {
		// Currently this is necessary for the keyboard commands
		// that change the screen rect. I don't understand things
		// well enough to know why this is a problem or what turning
		// it off could be doing, but everything LOOKS fine.
		mEngine.setToUserCamera();
	}

	// Verify that the application has included the framework resources.
	try {
		ci::DataSourceRef ds = loadResource(RES_ARC_DROPSHADOW);
	} catch (std::exception&) {
		std::cout << "ERROR Failed to load framework resource -- did you include FrameworkResources.rc in your application project?" << std::endl;
	}

	// Run all the statically-created initialization code.
	std::vector<std::function<void(ds::Engine&)>>& startups = get_startups();
	for (auto it=startups.begin(), end=startups.end(); it!=end; ++it) {
		if (*it) (*it)(mEngine);
	}
	startups.clear();
}

App::~App() {
	delete &(mEngine);
	ds::getLogger().shutDown();
	DS_DBG_CODE(GLOBAL_CONSOLE.destroy());
}

void App::prepareSettings(Settings *settings)
{
  inherited::prepareSettings(settings);

  if (settings) {
    mEngine.prepareSettings(*settings);

    // set in the engine.xml
    ci::Vec3f pos = mEngineSettings.getPoint("window_pos", 0, ci::Vec3f(0.0f, 0.0f, 0.0f));
    settings->setWindowPos(static_cast<unsigned>(pos.x), static_cast<unsigned>(pos.y));
  }
}

void App::setup()
{
  inherited::setup();

  mEngine.setup(*this);
  mEngine.setupTuio(*this);
}

void App::update()
{
  if (mEngine.hideMouse())
    hideCursor();
  mEngine.update();
}

void App::draw()
{
  mEngine.draw();
}

void App::mouseDown( MouseEvent event )
{
//	DS_LOG_INFO_M("App::mouseDown (" << event.getX() << "," << event.getY() << ")", ds::IO_LOG);
  if (mCtrlDown) {
    if (!mSecondMouseDown) {
      mEngine.mouseTouchBegin(event, 2);
      mSecondMouseDown = true;
    } else {
      mEngine.mouseTouchEnded(event, 2);
      mSecondMouseDown = false;
    }
  } else {
    mEngine.mouseTouchBegin(event, 1);
  }
}

void App::mouseMove( MouseEvent event )
{
//	DS_LOG_INFO_M("App::mouseMove (" << event.getX() << "," << event.getY() << ")", ds::IO_LOG);
}

void App::mouseDrag( MouseEvent event )
{
//	DS_LOG_INFO_M("App::mouseDrag (" << event.getX() << "," << event.getY() << ")", ds::IO_LOG);
  mEngine.mouseTouchMoved(event, 1);
}

void App::mouseUp( MouseEvent event )
{
//	DS_LOG_INFO_M("App::mouseUp (" << event.getX() << "," << event.getY() << ")", ds::IO_LOG);
  mEngine.mouseTouchEnded(event, 1);
}

void App::touchesBegan( TouchEvent event )
{
//	DS_LOG_INFO_M("App::touchesBegan", ds::IO_LOG);
  mEngine.touchesBegin(event);
}

void App::touchesMoved( TouchEvent event )
{
//	DS_LOG_INFO_M("App::touchesMoved", ds::IO_LOG);
  mEngine.touchesMoved(event);
}

void App::touchesEnded( TouchEvent event )
{
//	DS_LOG_INFO_M("App::touchesEnded", ds::IO_LOG);
  mEngine.touchesEnded(event);
}

void App::tuioObjectBegan(const TuioObject&) {
}

void App::tuioObjectMoved(const TuioObject&) {
}

void App::tuioObjectEnded(const TuioObject&) {
}

const std::string& App::envAppPath() {
	return APP_PATH;
}

const std::string& App::envAppDataPath() {
	return APP_DATA_PATH;
}

void App::keyDown(KeyEvent e) {
	const int		code = e.getCode();
	if ( ( mEscKeyEnabled && code == KeyEvent::KEY_ESCAPE ) || ( mQKeyEnabled && code == KeyEvent::KEY_q ) ) {
		std::exit(0);
	}
	if (code == KeyEvent::KEY_LCTRL || code == KeyEvent::KEY_RCTRL) {
		mCtrlDown = true;
	}
	if (mArrowKeyCameraControl) {
		if (code == KeyEvent::KEY_LEFT) {
			mEngineData.mScreenRect.x1 -= mArrowKeyCameraStep;
			mEngineData.mScreenRect.x2 -= mArrowKeyCameraStep;
			mEngineData.mCameraDirty = true;
		} else if (code == KeyEvent::KEY_RIGHT) {
			mEngineData.mScreenRect.x1 += mArrowKeyCameraStep;
			mEngineData.mScreenRect.x2 += mArrowKeyCameraStep;
			mEngineData.mCameraDirty = true;
		} else if (code == KeyEvent::KEY_UP) {
			mEngineData.mScreenRect.y1 -= mArrowKeyCameraStep;
			mEngineData.mScreenRect.y2 -= mArrowKeyCameraStep;
			mEngineData.mCameraDirty = true;
		} else if (code == KeyEvent::KEY_DOWN) {
			mEngineData.mScreenRect.y1 += mArrowKeyCameraStep;
			mEngineData.mScreenRect.y2 += mArrowKeyCameraStep;
			mEngineData.mCameraDirty = true;
		}
	}
}

void App::keyUp( KeyEvent event )
{
  if ( event.getCode() == KeyEvent::KEY_LCTRL || event.getCode() == KeyEvent::KEY_RCTRL )
    mCtrlDown = false;
}

void App::enableCommonKeystrokes( bool q /*= true*/, bool esc /*= true*/ ){
  if (q)
	mQKeyEnabled = true;
  if (esc)
    mEscKeyEnabled = true;
}

void App::quit()
{
  ci::app::AppBasic::quit();
}

void App::shutdown()
{
  mEngine.getRootSprite().clearChildren();
  ds::ui::clearFontCache();
  ci::app::AppBasic::shutdown();
}


/**
 * \class ds::App::Initializer
 */
static std::string data_folder_from(const Poco::Path& path) {
	Poco::Path          parent(path);
	Poco::Path			p(parent);
	p.append("data");
	const Poco::File    f(p);
	if (f.exists() && f.isDirectory()) {
		return parent.toString();
	}
	return "";
}

ds::App::Initializer::Initializer(const std::string& appPath) {
	DS_DBG_CODE(GLOBAL_CONSOLE.create());
	APP_PATH = appPath;

	Poco::Path      p(appPath);
	std::string     ans;
	// A couple things limit the search -- the directory can't get too
	// short, and right now nothing is more then 3 steps from the appPath
	// (but pad that to 5).
	int             count = 0;
	while ((ans=data_folder_from(p)).empty()) {
		p.popDirectory();
		if (count++ >= 5 || p.depth() < 2) break;
	}
	APP_DATA_PATH = ans;
}

} // namespace ds

static ds::Engine&    new_engine(	ds::App& app, const ds::cfg::Settings& settings,
									ds::EngineData& ed, const std::vector<int>* roots)
{
  if (settings.getText("platform:architecture", 0, "") == "client") return *(new ds::EngineClient(app, settings, ed, roots));
  if (settings.getText("platform:architecture", 0, "") == "server") return *(new ds::EngineServer(app, settings, ed, roots));
  return *(new ds::EngineClientServer(app, settings, ed, roots));
}
