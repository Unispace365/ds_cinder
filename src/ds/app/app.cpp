#include "ds/app/app.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include "ds/app/engine/engine_client.h"
#include "ds/app/engine/engine_clientserver.h"
#include "ds/app/engine/engine_server.h"
#include "ds/app/engine/engine_standalone.h"
#include "ds/app/engine/engine_stats_view.h"
#include "ds/app/environment.h"
#include "ds/debug/console.h"
#include "ds/debug/logger.h"
#include "ds/debug/debug_defines.h"
// For installing the sprite types
#include "ds/app/engine/engine_stats_view.h"
#include "ds/ui/sprite/gradient_sprite.h"
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/nine_patch.h"
#include "ds/ui/sprite/text.h"
// For installing the image generators
#include "ds/ui/image_source/image_arc.h"
#include "ds/ui/image_source/image_file.h"
#include "ds/ui/image_source/image_glsl.h"
#include "ds/ui/image_source/image_resource.h"
// For installing mesh caches
#include "ds/ui/mesh_source/mesh_cache_service.h"
// For installing the framework services
#include "ds/ui/service/glsl_image_service.h"
// For verifying that the resources are installed
#include "ds/app/FrameworkResources.h"

#ifdef _DEBUG
#include "AntTweakBar.h"
#endif

// Answer a new engine based on the current settings
static ds::Engine&    new_engine(ds::App&, const ds::cfg::Settings&, ds::EngineData&, const ds::RootList& roots);

static std::vector<std::function<void(ds::Engine&)>>& get_startups() {
	static std::vector<std::function<void(ds::Engine&)>>	VEC;
	return VEC;
}

namespace {
std::string				APP_PATH;
std::string				APP_DATA_PATH;

//#ifdef _DEBUG
ds::Console				GLOBAL_CONSOLE;
//#endif

void					add_dll_path() {
	// If there's a DLL folder, then add it to my PATH environment variable.
	try {
		if (APP_DATA_PATH.empty()) return;
		Poco::Path		p(APP_DATA_PATH);
		p.append("dll");
		if (Poco::File(p).exists()) {
			ds::Environment::addToEnvironmentVariable("PATH", p.toString());
		}
	} catch (std::exception const&) {
	}
}

}

namespace ds {

void App::AddStartup(const std::function<void(ds::Engine&)>& fn) {
	if (fn != nullptr) get_startups().push_back(fn);
}

/**
 * \class ds::App
 */
App::App(const RootList& roots)
	: mInitializer(getAppPath().generic_string())
	, mShowConsole(false)
	, mEngineSettings()
	, mEngineData(mEngineSettings)
	, mEngine(new_engine(*this, mEngineSettings, mEngineData, roots))
	, mCtrlDown(false)
	, mSecondMouseDown(false)
	, mQKeyEnabled(true)
	, mEscKeyEnabled(true)
	, mArrowKeyCameraStep(mEngineSettings.getFloat("camera:arrow_keys", 0, -1.0f))
	, mArrowKeyCameraControl(mArrowKeyCameraStep > 0.025f)
{
	add_dll_path();

	// Initialize each sprite type with a unique blob handler for network communication.
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Sprite::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Sprite::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Gradient::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Gradient::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Image::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Image::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::NinePatch::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::NinePatch::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Text::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Text::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){EngineStatsView::installAsServer(r);},
							[](ds::BlobRegistry& r){EngineStatsView::installAsClient(r);});

	// Initialize the engine image generator typess.
	ds::ui::ImageArc::install(mEngine.getImageRegistry());
	ds::ui::ImageFile::install(mEngine.getImageRegistry());
	ds::ui::ImageGlsl::install(mEngine.getImageRegistry());
	ds::ui::ImageResource::install(mEngine.getImageRegistry());

	// Install the framework services
	mEngine.addService(ds::glsl::IMAGE_SERVICE, *(new ds::glsl::ImageService(mEngine)));
	mEngine.addService(ds::MESH_CACHE_SERVICE_NAME, *(new ds::MeshCacheService()));

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

#ifdef _DEBUG
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(static_cast<int>(mEngine.getWidth()), static_cast<int>(mEngine.getHeight()));
#endif
}

App::~App() {
#ifdef _DEBUG
	TwTerminate();
#endif
	delete &(mEngine);
	ds::getLogger().shutDown();
	if(mShowConsole){
		GLOBAL_CONSOLE.destroy();
	}
}

void App::prepareSettings(Settings *settings) {
	inherited::prepareSettings(settings);

	if (settings) {
		mEngine.prepareSettings(*settings);
		settings->setWindowPos(static_cast<unsigned>(mEngineData.mDstRect.x1), static_cast<unsigned>(mEngineData.mDstRect.y1));
	}
}

void App::setup() {
	inherited::setup();

	mEngine.setup(*this);
	mEngine.setupTuio(*this);
}

void App::update() {
	if (mEngine.hideMouse()) {
		hideCursor();
	}
	mEngine.update();
}

void App::draw() {
	mEngine.draw();
#ifdef _DEBUG
	TwDraw(); //drawing after engine, on top of everything else
#endif // _DEBUG
}

void App::mouseDown( MouseEvent event ) {
#ifdef _DEBUG
	int isObserverHandling = 0;
	isObserverHandling += event.isLeftDown() ? TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT) : 0;
	isObserverHandling += event.isRightDown() ? TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_RIGHT) : 0;
	isObserverHandling += event.isMiddleDown() ? TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_MIDDLE) + TwMouseWheel(static_cast<int>(event.getWheelIncrement())) : 0;
	if (isObserverHandling > 0)
		return; //event handled by Observers
#endif
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

void App::mouseMove(MouseEvent e) {
#ifdef _DEBUG
	if (TwMouseMotion(static_cast<int>(e.getX()), static_cast<int>(e.getY())) == 1)
		return; //event handled by Observers
#endif
}

void App::mouseDrag(MouseEvent e) {
#ifdef _DEBUG
	int isObserverHandling = 0;
	isObserverHandling += e.isLeftDown() ? TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT) : 0;
	isObserverHandling += e.isRightDown() ? TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_RIGHT) : 0;
	isObserverHandling += e.isMiddleDown() ? TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_MIDDLE) + TwMouseWheel(static_cast<int>(e.getWheelIncrement())) : 0;
	isObserverHandling += TwMouseMotion(static_cast<int>(e.getX()), static_cast<int>(e.getY()));
	if (isObserverHandling > 0)
		return; //event handled by Observers
#endif
	mEngine.mouseTouchMoved(e, 1);
}

void App::mouseUp(MouseEvent e) {
#ifdef _DEBUG
	int isObserverHandling = 0;
	isObserverHandling += e.isLeft() ? TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT) : 0;
	isObserverHandling += e.isRight() ? TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_RIGHT) : 0;
	isObserverHandling += e.isMiddle() ? TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_MIDDLE) : 0;
	if (isObserverHandling > 0)
		return; //event handled by Observers
#endif
	mEngine.mouseTouchEnded(e, 1);
}

void App::touchesBegan(TouchEvent e) {
	mEngine.touchesBegin(e);
}

void App::touchesMoved(TouchEvent e) {
	mEngine.touchesMoved(e);
}

void App::touchesEnded(TouchEvent e) {
	mEngine.touchesEnded(e);
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

#ifdef _DEBUG

inline static bool routeKeyToObserver(KeyEvent &e) {
	int isHandled = 0;
	int modifier = TW_KMOD_NONE;
	if (e.isShiftDown())
		modifier |= TW_KMOD_SHIFT;
	if (e.isControlDown())
		modifier |= TW_KMOD_CTRL;
	if (e.isAltDown())
		modifier |= TW_KMOD_ALT;

	if (e.getCode() == KeyEvent::KEY_BACKSPACE) isHandled += TwKeyPressed(TW_KEY_BACKSPACE, modifier);
	else if (e.getCode() == KeyEvent::KEY_TAB) isHandled += TwKeyPressed(TW_KEY_TAB, modifier);
	else if (e.getCode() == KeyEvent::KEY_CLEAR) isHandled += TwKeyPressed(TW_KEY_CLEAR, modifier);
	else if (e.getCode() == KeyEvent::KEY_RETURN) isHandled += TwKeyPressed(TW_KEY_RETURN, modifier);
	else if (e.getCode() == KeyEvent::KEY_PAUSE) isHandled += TwKeyPressed(TW_KEY_PAUSE, modifier);
	else if (e.getCode() == KeyEvent::KEY_ESCAPE) isHandled += TwKeyPressed(TW_KEY_ESCAPE, modifier);
	else if (e.getCode() == KeyEvent::KEY_SPACE) isHandled += TwKeyPressed(TW_KEY_SPACE, modifier);
	else if (e.getCode() == KeyEvent::KEY_DELETE) isHandled += TwKeyPressed(TW_KEY_DELETE, modifier);
	else if (e.getCode() == KeyEvent::KEY_UP) isHandled += TwKeyPressed(TW_KEY_UP, modifier);
	else if (e.getCode() == KeyEvent::KEY_DOWN) isHandled += TwKeyPressed(TW_KEY_DOWN, modifier);
	else if (e.getCode() == KeyEvent::KEY_RIGHT) isHandled += TwKeyPressed(TW_KEY_RIGHT, modifier);
	else if (e.getCode() == KeyEvent::KEY_LEFT) isHandled += TwKeyPressed(TW_KEY_LEFT, modifier);
	else if (e.getCode() == KeyEvent::KEY_INSERT) isHandled += TwKeyPressed(TW_KEY_INSERT, modifier);
	else if (e.getCode() == KeyEvent::KEY_HOME) isHandled += TwKeyPressed(TW_KEY_HOME, modifier);
	else if (e.getCode() == KeyEvent::KEY_END) isHandled += TwKeyPressed(TW_KEY_END, modifier);
	else if (e.getCode() == KeyEvent::KEY_PAGEUP) isHandled += TwKeyPressed(TW_KEY_PAGE_UP, modifier);
	else if (e.getCode() == KeyEvent::KEY_PAGEDOWN) isHandled += TwKeyPressed(TW_KEY_PAGE_DOWN, modifier);
	else if (e.getCode() == KeyEvent::KEY_F1) isHandled += TwKeyPressed(TW_KEY_F1, modifier);
	else if (e.getCode() == KeyEvent::KEY_F2) isHandled += TwKeyPressed(TW_KEY_F2, modifier);
	else if (e.getCode() == KeyEvent::KEY_F3) isHandled += TwKeyPressed(TW_KEY_F3, modifier);
	else if (e.getCode() == KeyEvent::KEY_F4) isHandled += TwKeyPressed(TW_KEY_F4, modifier);
	else if (e.getCode() == KeyEvent::KEY_F5) isHandled += TwKeyPressed(TW_KEY_F5, modifier);
	else if (e.getCode() == KeyEvent::KEY_F6) isHandled += TwKeyPressed(TW_KEY_F6, modifier);
	else if (e.getCode() == KeyEvent::KEY_F7) isHandled += TwKeyPressed(TW_KEY_F7, modifier);
	else if (e.getCode() == KeyEvent::KEY_F8) isHandled += TwKeyPressed(TW_KEY_F8, modifier);
	else if (e.getCode() == KeyEvent::KEY_F9) isHandled += TwKeyPressed(TW_KEY_F9, modifier);
	else if (e.getCode() == KeyEvent::KEY_F10) isHandled += TwKeyPressed(TW_KEY_F10, modifier);
	else if (e.getCode() == KeyEvent::KEY_F11) isHandled += TwKeyPressed(TW_KEY_F11, modifier);
	else if (e.getCode() == KeyEvent::KEY_F12) isHandled += TwKeyPressed(TW_KEY_F12, modifier);
	else if (e.getCode() == KeyEvent::KEY_F13) isHandled += TwKeyPressed(TW_KEY_F13, modifier);
	else if (e.getCode() == KeyEvent::KEY_F14) isHandled += TwKeyPressed(TW_KEY_F14, modifier);
	else if (e.getCode() == KeyEvent::KEY_F15) isHandled += TwKeyPressed(TW_KEY_F15, modifier);
	else isHandled += TwKeyPressed(e.getChar(), modifier);

	return isHandled > 0 ? true : false;
}

#endif // _DEBUG

void App::keyDown(KeyEvent e) {
	const int		code = e.getCode();

#ifdef _DEBUG
	if (routeKeyToObserver(e))
		return;
#endif // _DEBUG

	if ( ( mEscKeyEnabled && code == KeyEvent::KEY_ESCAPE ) || ( mQKeyEnabled && code == KeyEvent::KEY_q ) ) {
		quit();
	}
	if (code == KeyEvent::KEY_LCTRL || code == KeyEvent::KEY_RCTRL) {
		mCtrlDown = true;
	} else if (KeyEvent::KEY_s == code) {
		mEngine.getNotifier().notify(EngineStatsView::Toggle());
	} else if (KeyEvent::KEY_t == code) {
		mEngine.nextTouchMode();
	}

	if (mArrowKeyCameraControl) {
		if (code == KeyEvent::KEY_LEFT) {
			mEngineData.mScreenRect.x1 -= mArrowKeyCameraStep;
			mEngineData.mScreenRect.x2 -= mArrowKeyCameraStep;
			mEngine.markCameraDirty();
		} else if (code == KeyEvent::KEY_RIGHT) {
			mEngineData.mScreenRect.x1 += mArrowKeyCameraStep;
			mEngineData.mScreenRect.x2 += mArrowKeyCameraStep;
			mEngine.markCameraDirty();
		} else if (code == KeyEvent::KEY_UP) {
			mEngineData.mScreenRect.y1 -= mArrowKeyCameraStep;
			mEngineData.mScreenRect.y2 -= mArrowKeyCameraStep;
			mEngine.markCameraDirty();
		} else if (code == KeyEvent::KEY_DOWN) {
			mEngineData.mScreenRect.y1 += mArrowKeyCameraStep;
			mEngineData.mScreenRect.y2 += mArrowKeyCameraStep;
			mEngine.markCameraDirty();
		}
	}

#ifdef _DEBUG
	if (code == KeyEvent::KEY_d) {
		std::string		path = ds::Environment::expand("%LOCAL%/sprite_dump.txt");
		std::cout << "WRITING OUT SPRITE HIERARCHY (" << path << ")" << std::endl;
	    std::fstream	filestr;
	    filestr.open(path, std::fstream::out);
	    if (filestr.is_open()) {
			mEngine.writeSprites(filestr);
			filestr.close();
	    }
		// and to console
		std::stringstream		buf;
		mEngine.writeSprites(buf);
		std::cout << buf.str() << std::endl;
	}
#endif
}

void App::keyUp( KeyEvent event )
{
  if ( event.getCode() == KeyEvent::KEY_LCTRL || event.getCode() == KeyEvent::KEY_RCTRL )
    mCtrlDown = false;
}

void App::enableCommonKeystrokes( bool q /*= true*/, bool esc /*= true*/ ) {
	if (q) {
		mQKeyEnabled = q;
	}
	if (esc) {
		mEscKeyEnabled = esc;
	}
}

void App::quit() {
	ci::app::AppBasic::quit();
}

void App::shutdown() {
	mEngine.getRootSprite().clearChildren();
	mEngine.stopServices();
	ds::ui::clearFontCache();
	ci::app::AppBasic::shutdown();
}

void App::showConsole(){
	// prevent calling create multiple times
	if(mShowConsole) return;

	mShowConsole = true;
	GLOBAL_CONSOLE.create();
}


/**
 * \class ds::App::Initializer
 */
static std::string app_sub_folder_from(const std::string &sub, const Poco::Path &path) {
	Poco::Path          parent(path);
	Poco::Path			p(parent);
	p.append(sub);
	const Poco::File    f(p);
	if (f.exists() && f.isDirectory()) {
		return parent.toString();
	}
	return "";
}

static std::string app_folder_from(const Poco::Path& path) {
	// Look for either a "data" or "settings" folder; either indicates I'm in the right place.
	std::string			fn(app_sub_folder_from("data", path));
	if (!fn.empty()) return fn;
	return app_sub_folder_from("settings", path);
}

ds::App::Initializer::Initializer(const std::string& appPath) {
	APP_PATH = appPath;

	Poco::Path      p(appPath);
	std::string     ans;
	// A couple things limit the search -- the directory can't get too
	// short, and right now nothing is more then 3 steps from the appPath
	// (but pad that to 5).
	int             count = 0;
	while ((ans=app_folder_from(p)).empty()) {
		p.popDirectory();
		if (count++ >= 5 || p.depth() < 2) break;
	}
	APP_DATA_PATH = ans;
}

} // namespace ds

static ds::Engine&    new_engine(	ds::App& app, const ds::cfg::Settings& settings,
									ds::EngineData& ed, const ds::RootList& roots) {

	bool defaultShowConsole = false;
	DS_DBG_CODE(defaultShowConsole = true);
	if(settings.getBool("console:show", 0, defaultShowConsole)){
		app.showConsole();
	}

	const std::string	arch(settings.getText("platform:architecture", 0, ""));
	if (arch == "client") return *(new ds::EngineClient(app, settings, ed, roots));
	if (arch == "server") return *(new ds::EngineServer(app, settings, ed, roots));
	if (arch == "clientserver") return *(new ds::EngineClientServer(app, settings, ed, roots));
	return *(new ds::EngineStandalone(app, settings, ed, roots));
}
