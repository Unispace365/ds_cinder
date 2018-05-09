#include "stdafx.h"

#include "ds/app/app.h"

#include <Poco/File.h>
#include <Poco/Path.h>

#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_client.h"
#include "ds/app/engine/engine_clientserver.h"
#include "ds/app/engine/engine_server.h"
#include "ds/app/engine/engine_standalone.h"
#include "ds/app/engine/engine_stats_view.h"
#include "ds/app/environment.h"
#include "ds/debug/logger.h"
#include "ds/debug/debug_defines.h"
#include "ds/content/content_events.h"

// For installing the sprite types
#include "ds/app/engine/engine_stats_view.h"
#include "ds/ui/soft_keyboard/entry_field.h"
#include "ds/ui/sprite/gradient_sprite.h"
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/text.h"
#include "ds/ui/sprite/border.h"
#include "ds/ui/sprite/circle.h"
#include "ds/ui/sprite/circle_border.h"

// For the screenshot
#include <Poco/Timestamp.h>
#include <Poco/Path.h>
#include <cinder/ip/Flip.h>
#include <cinder/ImageIo.h>

#ifndef _WIN32
// For access to Linux native GLFW window calls
#include "glfw/glfw3.h"
#include "glfw/glfw3native.h"
// For Linux window file-drop event registration
#include <cinder/app/FileDropEvent.h>
#include <cinder/Filesystem.h>

// Add a file-drop handler under Linux, since Cinder's Linux implementation currently doesn't support this
namespace {
void linuxImplRegisterWindowFiledropHandler( ci::app::WindowRef cinderWindow ) {
	static ci::app::WindowRef sMainAppCinderWindow = cinderWindow;

	::glfwSetDropCallback( (GLFWwindow*)sMainAppCinderWindow->getNative(), [](GLFWwindow *window, int numPaths, const char **paths) {
		if( sMainAppCinderWindow && (GLFWwindow*)sMainAppCinderWindow->getNative() == window ) {
			DS_LOG_INFO( "Dropped files on window: " << window );
			std::vector<ci::fs::path> files;
			for (int i=0; i<numPaths; i++) {
				DS_LOG_INFO( "  " << i << ": " << std::string(paths[i]) );
				files.push_back( std::string( paths[i] ) );
			}

			ci::app::FileDropEvent dropEvent( sMainAppCinderWindow, 0, 0, files );
			sMainAppCinderWindow->emitFileDrop( &dropEvent );
		}
	});
}
} //anonymous namespace
#endif // !_WIN32

// Answer a new engine based on the current settings
static ds::Engine&    new_engine(ds::App&, ds::EngineSettings&, ds::EngineData&, const ds::RootList& roots);

static std::vector<std::function<void(ds::Engine&)>>& get_startups() {
	static std::vector<std::function<void(ds::Engine&)>>	VEC;
	return VEC;
}

static std::vector<std::function<void(ds::Engine&)>>& get_setups() {
	static std::vector<std::function<void(ds::Engine&)>>	VEC;
	return VEC;
}

namespace {
std::string				APP_DATA_PATH;

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

} // anonymous namespace

namespace ds {

EngineSettingsPreloader::EngineSettingsPreloader( ci::app::AppBase::Settings* settings )
		: mInitializer()
		, mEngineSettings()
	{
		earlyPrepareAppSettings( settings );
	}


void EngineSettingsPreloader::earlyPrepareAppSettings( ci::app::AppBase::Settings* settings ) {
	// Enable MultiTouch on app window if needed
	const auto touchMode = ds::ui::TouchMode::fromSettings(mEngineSettings);
	if(ds::ui::TouchMode::hasSystem(ds::ui::TouchMode::fromSettings(mEngineSettings))) {
		settings->setMultiTouchEnabled();
	}
}


void App::AddStartup(const std::function<void(ds::Engine&)>& fn) {
	if(fn != nullptr) get_startups().push_back(fn);
}

void App::AddServerSetup(const std::function<void(ds::Engine&)>& fn) {
	if(fn != nullptr) get_setups().push_back(fn);
}

/**
 * \class ds::App
 */
App::App(const RootList& roots)
	: EngineSettingsPreloader( ci::app::AppBase::sSettingsFromMain )
	, ci::app::App()
	, mEnvironmentInitialized(ds::Environment::initialize())
	, mEngineData(mEngineSettings)
	, mEngine(new_engine(*this, mEngineSettings, mEngineData, roots))
	, mTouchDebug(mEngine)
	, mAppKeysEnabled(true)
	, mMouseHidden(false)
	, mArrowKeyCameraStep(mEngineSettings.getFloat("camera:arrow_keys"))
	, mArrowKeyCameraControl(mArrowKeyCameraStep > 0.025f)
{

	setupKeyPresses();

	mEngineSettings.printStartupInfo();

	add_dll_path();

	// Initialize each sprite type with a unique blob handler for network communication.
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Sprite::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Sprite::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Gradient::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Gradient::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Image::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Image::installAsClient(r);});
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Text::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Text::installAsClient(r); });
	mEngine.installSprite(	[](ds::BlobRegistry& r){EngineStatsView::installAsServer(r);},
							[](ds::BlobRegistry& r){EngineStatsView::installAsClient(r);});
	mEngine.installSprite(  [](ds::BlobRegistry& r){ds::ui::Border::installAsServer(r); },
							[](ds::BlobRegistry& r){ds::ui::Border::installAsClient(r); });
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::Circle::installAsServer(r); },
							[](ds::BlobRegistry& r){ds::ui::Circle::installAsClient(r); });
	mEngine.installSprite(	[](ds::BlobRegistry& r){ds::ui::CircleBorder::installAsServer(r); },
				  			[](ds::BlobRegistry& r){ds::ui::CircleBorder::installAsClient(r); });

	// Run all the statically-created initialization code.
	std::vector<std::function<void(ds::Engine&)>>& startups = get_startups();
	for (auto it=startups.begin(), end=startups.end(); it!=end; ++it) {
		if (*it) (*it)(mEngine);
	}
	startups.clear();

	setFpsSampleInterval(0.25);

	prepareSettings(ci::app::App::get()->sSettingsFromMain);

	DS_LOG_INFO(mEngine.getAppInstanceName() << " startup");
	mEngine.recordMetric("engine", "startup", 1);
}

App::~App() {
	mEngine.recordMetric("engine", "shutdown", 1);
	DS_LOG_INFO(mEngine.getAppInstanceName() << " shutting down");

	delete &(mEngine);
	ds::getLogger().shutDown();
}

void App::prepareSettings(ci::app::AppBase::Settings *settings) {

	if (settings) {
		mEngine.prepareSettings(*settings);
		settings->setWindowPos(static_cast<unsigned>(mEngineData.mDstRect.x1), static_cast<unsigned>(mEngineData.mDstRect.y1));
		inherited::setFrameRate(settings->getFrameRate());
		inherited::setWindowSize(settings->getWindowSize());
		inherited::setWindowPos(settings->getWindowPos());
		inherited::setFullScreen(settings->isFullScreen());
		inherited::getWindow()->setBorderless(settings->isBorderless());
		inherited::getWindow()->setAlwaysOnTop(settings->isAlwaysOnTop());
		inherited::getWindow()->setTitle(settings->getTitle());
		inherited::enablePowerManagement(settings->isPowerManagementEnabled());

#ifndef _WIN32
		auto window = (GLFWwindow*) inherited::getWindow()->getNative();
		if (settings->isFullScreen()) {
			GLFWmonitor* monitor = ::glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = ::glfwGetVideoMode(monitor);
			::glfwSetWindowMonitor( window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate );
		}
#endif
	}
#ifndef _WIN32
	linuxImplRegisterWindowFiledropHandler(inherited::getWindow());
#endif // !_WIN32


	loadAppSettings();
}

void App::loadAppSettings() {
	// Fonts links together a font name and a physical font file
	// Then the "text.xml" and TextCfg will use those font names to specify visible settings (size, color, leading)
	mEngine.loadSettings("fonts", "fonts.xml");
	mEngine.editFonts().clear();
	mEngine.getSettings("fonts").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting) {
		mEngine.editFonts().installFont(ds::Environment::expand(theSetting.mRawValue), theSetting.mName, theSetting.mName);
	}, ds::cfg::SETTING_TYPE_STRING);

	// Colors
	// After registration, colors can be called by name from settings files or in the app
	mEngine.editColors().clear();
	mEngine.editColors().install(ci::Color(1.0f, 1.0f, 1.0f), "white");
	mEngine.editColors().install(ci::Color(0.0f, 0.0f, 0.0f), "black");
	mEngine.loadSettings("colors", "colors.xml");
	mEngine.getSettings("colors").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting) {
		mEngine.editColors().install(theSetting.getColorA(mEngine), theSetting.mName);
	}, ds::cfg::SETTING_TYPE_COLOR);

	/* Settings */
	mEngine.loadSettings("app_settings", "app_settings.xml");
	mEngine.loadTextCfg("text.xml");
}

void App::setup() {
	inherited::setup();

	mEngine.getPangoFontService().loadFonts();
	mEngine.setup(*this);
	mEngine.setupTouch(*this);

#ifdef _WIN32
	::SetForegroundWindow((HWND)ci::app::getWindow()->getNative());
#endif

	mEngine.getLoadImageService().initialize();
}

void App::resetupServer() {
	mEngine.clearAllSprites(true);
	loadAppSettings();
	mEngine.reloadSettings();
	mEngine.getLoadImageService().initialize();
	//setupServer();
}

void App::preServerSetup() {
	for(auto it : get_setups()) {
		it(mEngine);
	}
}

void App::update() {
#ifdef _WIN32
	if(mEngine.getEngineSettings().getBool("system:never_sleep", 0, true)) {
		// prevents the system from going to sleep
		SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED | ES_AWAYMODE_REQUIRED);
	}
#endif

	mEngine.setAverageFps(getAverageFps());

	DS_LOG_VERBOSE(9, "App::Update fps=" << getAverageFps());

	if (mEngine.getHideMouse() && !mMouseHidden) {
		mMouseHidden = true;
		hideCursor();
	} else if(mMouseHidden && !mEngine.getHideMouse()){
		mMouseHidden = false;
		showCursor();
	}

	if(mEngine.getAutoHideMouse() && !mEngine.getHideMouse()) {
		auto nowwy = Poco::Timestamp().epochMicroseconds();
		if((float)(nowwy - mMouseMoveTime) / 1000000.0f > 2.0f) {
			mEngine.setHideMouse(true);
		}
	}
	mEngine.update();

	if(mEngine.getRestartAfterNextUpdate() && mEngine.getMode() != ds::ui::SpriteEngine::CLIENT_MODE) {
		resetupServer();
	}
}

void App::draw() {
	mEngine.draw();
}
void App::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void App::mouseMove(ci::app::MouseEvent e) {
	if(mEngine.getAutoHideMouse()) {
		mMouseMoveTime = Poco::Timestamp().epochMicroseconds();
		mEngine.setHideMouse(false);
	}
}

void App::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void App::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void App::touchesBegan(ci::app::TouchEvent e) {
	if(mEngine.getAutoHideMouse()) {
		mEngine.setHideMouse(true);
	}
	mEngine.touchesBegin(e);
}

void App::touchesMoved(ci::app::TouchEvent e) {
	mEngine.touchesMoved(e);
}

void App::touchesEnded(ci::app::TouchEvent e) {
	mEngine.touchesEnded(e);
}

void App::tuioObjectBegan(const ds::TuioObject&) {
}

void App::tuioObjectMoved(const ds::TuioObject&) {
}

void App::tuioObjectEnded(const ds::TuioObject&) {
}

const std::string& App::envAppDataPath() {
	return APP_DATA_PATH;
}

void App::killSupportingApps() {
	DS_LOG_INFO("App: killing supporting apps then myself");
	system("taskkill /f /im RestartOnCrash.exe");
	system("taskkill /f /im DSNode-Host.exe");
	system("taskkill /f /im DSNodeConsole.exe");
	system("taskkill /f /im DSNode.exe");
	system("taskkill /f /im DSAppHost.exe");
	quit();
}

void App::writeSpriteHierarchy() {
	std::string		path = ds::Environment::expand("%LOCAL%/sprite_dump.txt");
	std::cout << "WRITING OUT SPRITE HIERARCHY (" << path << ")" << std::endl;
	std::fstream	filestr;
	filestr.open(path, std::fstream::out);
	if(filestr.is_open()) {
		mEngine.writeSprites(filestr);
		filestr.close();
	}
	// and to console
	std::stringstream		buf;
	mEngine.writeSprites(buf);
	std::cout << buf.str() << std::endl;
}

void App::debugEnabledSprites() {
	DS_LOG_VERBOSE(1, "App::debugEnabledSprites()");
	const size_t numRoots = mEngine.getRootCount();
	int numPlacemats = 0;
	for(size_t i = 0; i < numRoots - 1; i++) {
		mEngine.getRootSprite(i).forEachChild([this](ds::ui::Sprite& sprite) {
			if(sprite.isEnabled()) {
				sprite.setTransparent(false);
				sprite.setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
				sprite.setOpacity(0.95f);

				ds::ui::Text* labelly = new ds::ui::Text(mEngine);
				labelly->setFont("Arial");
				labelly->setFontSize(16.0f);
				labelly->setText(typeid(sprite).name());
				labelly->enable(false);
				labelly->setColor(ci::Color::black());
			} else {

				ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(&sprite);
				if(!texty || (texty && texty->getColor() != ci::Color::black())) sprite.setTransparent(true);
			}
		}, true);
	}
}

void App::registerKeyPress(const std::string& name, std::function<void()> func, const int keyCode, const bool shiftDown /*= false*/, const bool ctrlDown /*= false*/, const bool altDown /*= false*/) {
	mKeyManager.registerKey(name, func, keyCode, shiftDown, ctrlDown, altDown);
}

void App::setupKeyPresses() {
	using ci::app::KeyEvent;
	mKeyManager.registerKey("Quit app", [this] { quit(); }, KeyEvent::KEY_ESCAPE);
	mKeyManager.registerKey("Quit app", [this] { quit(); }, KeyEvent::KEY_q);
	mKeyManager.registerKey("Quit app", [this] { quit(); }, KeyEvent::KEY_q, true);
	mKeyManager.registerKey("Quit app", [this] { quit(); }, KeyEvent::KEY_q, false, true);
	mKeyManager.registerKey("Quit app", [this] { quit(); }, KeyEvent::KEY_F4);
	mKeyManager.registerKey("Print available keys", [this] { mKeyManager.printCurrentKeys(); mEngine.getNotifier().notify(EngineStatsView::ToggleHelpRequest()); }, KeyEvent::KEY_h);
	mKeyManager.registerKey("Toggle stats", [this] {mEngine.getNotifier().notify(EngineStatsView::ToggleStatsRequest()); }, KeyEvent::KEY_s);
	mKeyManager.registerKey("Toggle fullscreen", [this] {setFullScreen(!isFullScreen()); }, KeyEvent::KEY_f);
	mKeyManager.registerKey("Toggle always on top", [this] {ci::app::getWindow()->setAlwaysOnTop(!ci::app::getWindow()->isAlwaysOnTop()); }, KeyEvent::KEY_a);
	mKeyManager.registerKey("Toggle idling", [this] {mEngine.isIdling() ? mEngine.resetIdleTimeout() : mEngine.startIdling(); }, KeyEvent::KEY_i);
	mKeyManager.registerKey("Toggle console", [this] {mEngine.toggleConsole(); }, KeyEvent::KEY_c);
	mKeyManager.registerKey("Touch mode", [this] {mEngine.nextTouchMode(); }, KeyEvent::KEY_t);
	mKeyManager.registerKey("Take screenshot", [this] {saveTransparentScreenshot(); }, KeyEvent::KEY_F8);
	mKeyManager.registerKey("Kill supporting apps", [this] { killSupportingApps(); }, KeyEvent::KEY_k, false, true);
	mKeyManager.registerKey("Toggle mouse", [this] { mEngine.setHideMouse(!mEngine.getHideMouse()); }, KeyEvent::KEY_m);
	mKeyManager.registerKey("Verbose logging toggle", [this] { if(ds::getLogger().getVerboseLevel() > 0) ds::getLogger().setVerboseLevel(0); else ds::getLogger().setVerboseLevel(9); }, KeyEvent::KEY_v);
	mKeyManager.registerKey("Verbose logging increment", [this] { ds::getLogger().incrementVerboseLevel(); }, KeyEvent::KEY_v, false, false, true);
	mKeyManager.registerKey("Verbose logging decrement", [this] { ds::getLogger().decrementVerboseLevel(); }, KeyEvent::KEY_v, true, false, true);
	mKeyManager.registerKey("Settings editor", [this] { mEngine.isShowingSettingsEditor() ? mEngine.hideSettingsEditor() : mEngine.showSettingsEditor(mEngineSettings); }, KeyEvent::KEY_e);
	mKeyManager.registerKey("Debug enabled sprites", [this] { debugEnabledSprites(); }, KeyEvent::KEY_d);
	mKeyManager.registerKey("Log sprite hierarchy", [this] { writeSpriteHierarchy(); }, KeyEvent::KEY_d, false, true);
	mKeyManager.registerKey("Log image cache", [this] { mEngine.getLoadImageService().logCache(); }, KeyEvent::KEY_g);
	mKeyManager.registerKey("Clear image cache", [this] { mEngine.getLoadImageService().clearCache(); }, KeyEvent::KEY_g, true);
	mKeyManager.registerKey("Requery data", [this] { mEngine.getNotifier().notify(ds::RequestContentQueryEvent()); }, ci::app::KeyEvent::KEY_n);
	mKeyManager.registerKey("Print data tree", [this] { mEngine.mContent.printTree(false, ""); }, ci::app::KeyEvent::KEY_l);
	mKeyManager.registerKey("Print data tree verbose", [this] { mEngine.mContent.printTree(true, ""); }, ci::app::KeyEvent::KEY_l, true);
	mKeyManager.registerKey("Log available font families", [this] { mEngine.getPangoFontService().logFonts(false); }, KeyEvent::KEY_p);
	mKeyManager.registerKey("Log all available fonts", [this] { mEngine.getPangoFontService().logFonts(true); }, KeyEvent::KEY_p, true);
	mKeyManager.registerKey("Restart app", [this] { resetupServer(); }, KeyEvent::KEY_r);

	mKeyManager.registerKey("Move src rect left", [this] {
		mEngineData.mSrcRect.x1 -= mArrowKeyCameraStep;
		mEngineData.mSrcRect.x2 -= mArrowKeyCameraStep;
		mEngine.markCameraDirty();
	}, KeyEvent::KEY_LEFT);

	mKeyManager.registerKey("Move src rect right", [this] {
		mEngineData.mSrcRect.x1 += mArrowKeyCameraStep;
		mEngineData.mSrcRect.x2 += mArrowKeyCameraStep;
		mEngine.markCameraDirty();
	}, KeyEvent::KEY_RIGHT);

	mKeyManager.registerKey("Move src rect up", [this] {
		mEngineData.mSrcRect.y1 -= mArrowKeyCameraStep;
		mEngineData.mSrcRect.y2 -= mArrowKeyCameraStep;
		mEngine.markCameraDirty();
	}, KeyEvent::KEY_UP);

	mKeyManager.registerKey("Move src rect down", [this] {
		mEngineData.mSrcRect.y1 += mArrowKeyCameraStep;
		mEngineData.mSrcRect.y2 += mArrowKeyCameraStep;
		mEngine.markCameraDirty();
	}, KeyEvent::KEY_DOWN);

}

void App::keyDown(ci::app::KeyEvent e) {
	DS_LOG_VERBOSE(3, "App::keyDown char=" << e.getChar() << " code=" << e.getCode());

	if(!mAppKeysEnabled){
		onKeyDown(e);
		return;
	}

	if(mEngine.getRegisteredEntryField()){
		mEngine.getRegisteredEntryField()->keyPressed(e);
		return;
	}

	if(mKeyManager.keyDown(e)) return;

	onKeyDown(e);
}

void App::keyUp(ci::app::KeyEvent e) {
	DS_LOG_VERBOSE(3, "App::keyUp char=" << e.getChar() << " code=" << e.getCode());
	onKeyUp(e);
}

void App::saveTransparentScreenshot() {
	DS_LOG_VERBOSE(1, "App::saveTransparentScreenshot()");

	Poco::Path		p(Poco::Path::home());
	Poco::Timestamp::TimeVal t = Poco::Timestamp().epochMicroseconds();
	std::stringstream filepath;
	filepath << "ds_cinder.screenshot." << t << ".png";
	p.append("Desktop").append(filepath.str());
	ci::writeImage(Poco::Path::expand(p.toString()), copyWindowSurface());
}

void App::quit(){
	ci::app::App::quit();
}

void App::shutdown(){
	quit();
}

/**
 * \class ds::EngineSettingsPreloader::Initializer
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

ds::EngineSettingsPreloader::Initializer::Initializer() {
	const auto appPath = ci::app::Platform::get()->getExecutablePath().generic_string();

	// appPath could contain a trailing slash (Windows), or not (Linux).
	// We need to parse it as a directory in either case
	Poco::Path      p = Poco::Path::forDirectory(appPath);
	std::string     ans;
	// A couple things limit the search -- the directory can't get too
	// short, and right now nothing is more then 3 steps from the appPath
	// (but pad that to 5).
	int             count = 0;
	while ((ans=app_folder_from(p)).empty()) {
		p.popDirectory();
		if (count++ >= 5 || p.depth() < 1) break;
	}
	APP_DATA_PATH = ans;
}

} // namespace ds

static ds::Engine&    new_engine(	ds::App& app, ds::EngineSettings& settings,
									ds::EngineData& ed, const ds::RootList& roots){
	const std::string	arch(settings.getString("platform:architecture", 0, ""));
	if (arch == "client") return *(new ds::EngineClient(app, settings, ed, roots));
	if (arch == "server") return *(new ds::EngineServer(app, settings, ed, roots));
	if (arch == "clientserver") return *(new ds::EngineClientServer(app, settings, ed, roots));
	return *(new ds::EngineStandalone(app, settings, ed, roots));
}
