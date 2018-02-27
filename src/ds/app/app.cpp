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

// For installing the sprite types
#include "ds/app/engine/engine_stats_view.h"
#include "ds/ui/soft_keyboard/entry_field.h"
#include "ds/ui/sprite/gradient_sprite.h"
#include "ds/ui/sprite/image.h"
#include "ds/ui/sprite/text.h"
#include "ds/ui/sprite/border.h"
#include "ds/ui/sprite/circle.h"
#include "ds/ui/sprite/circle_border.h"

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
	, mCtrlDown(false)
	, mTouchDebug(mEngine)
	, mAppKeysEnabled(true)
	, mMouseHidden(false)
	, mArrowKeyCameraStep(mEngineSettings.getFloat("camera:arrow_keys"))
	, mArrowKeyCameraControl(mArrowKeyCameraStep > 0.025f)
{

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

	// Initialize the engine image generator typess.
	ds::ui::ImageArc::install(mEngine.getImageRegistry());
	ds::ui::ImageFile::install(mEngine.getImageRegistry());
	ds::ui::ImageGlsl::install(mEngine.getImageRegistry());
	ds::ui::ImageResource::install(mEngine.getImageRegistry());

	// Install the framework services
	mEngine.addService(ds::glsl::IMAGE_SERVICE, *(new ds::glsl::ImageService(mEngine)));
	mEngine.addService(ds::MESH_CACHE_SERVICE_NAME, *(new ds::MeshCacheService()));

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

	setFpsSampleInterval(0.25);

	prepareSettings(ci::app::App::get()->sSettingsFromMain);


}

App::~App() {
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
	mEngine.loadSettings("FONTS", "fonts.xml");
	mEngine.editFonts().clear();
	mEngine.getSettings("FONTS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting) {
		mEngine.editFonts().installFont(ds::Environment::expand(theSetting.mRawValue), theSetting.mName, theSetting.mName);
	}, ds::cfg::SETTING_TYPE_STRING);

	// Colors
	// After registration, colors can be called by name from settings files or in the app
	mEngine.editColors().clear();
	mEngine.editColors().install(ci::Color(1.0f, 1.0f, 1.0f), "white");
	mEngine.editColors().install(ci::Color(0.0f, 0.0f, 0.0f), "black");
	mEngine.loadSettings("COLORS", "colors.xml");
	mEngine.getSettings("COLORS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting) {
		mEngine.editColors().install(theSetting.getColorA(mEngine), theSetting.mName);
	}, ds::cfg::SETTING_TYPE_COLOR);

	/* Settings */
	mEngine.loadSettings("APP", "app_settings.xml");
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
}

void App::resetupServer() {
	mEngine.clearAllSprites(true);
	loadAppSettings();
	mEngine.reloadSettings();
	//setupServer();
}

void App::preServerSetup() {
	for(auto it : get_setups()) {
		it(mEngine);
	}
}

void App::update() {
#ifdef _WIN32
	if(mEngine.getSettings("engine").getBool("system:never_sleep", 0, true)) {
		// prevents the system from going to sleep
		SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
	}
#endif

	mEngine.setAverageFps(getAverageFps());
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

void App::keyDown(ci::app::KeyEvent e) {
	if(!mAppKeysEnabled){
		onKeyDown(e);
		return;
	}

	if(mEngine.getRegisteredEntryField()){
		mEngine.getRegisteredEntryField()->keyPressed(e);
		return;
	}

	using ci::app::KeyEvent;
	const int		code = e.getCode();
	if(code == KeyEvent::KEY_ESCAPE || code == KeyEvent::KEY_q) {
		quit();
	}
	if(code == ci::app::KeyEvent::KEY_LCTRL || code == KeyEvent::KEY_RCTRL) {
		mCtrlDown = true;
	} else if(KeyEvent::KEY_s == code) {
		mEngine.getNotifier().notify(EngineStatsView::ToggleStatsRequest());
	} else if(KeyEvent::KEY_t == code) {
		mEngine.nextTouchMode();
	} else if(KeyEvent::KEY_F8 == code){
		saveTransparentScreenshot();
	} else if(KeyEvent::KEY_k == code && mCtrlDown){
		system("taskkill /f /im RestartOnCrash.exe");
		system("taskkill /f /im DSNode-Host.exe");
		system("taskkill /f /im DSNodeConsole.exe");
	} else if(ci::app::KeyEvent::KEY_m == code){
		mEngine.setHideMouse(!mEngine.getHideMouse());
	} else if(code == KeyEvent::KEY_v && e.isShiftDown()){
		mEngine.getTouchManager().setVerboseLogging(!mEngine.getTouchManager().getVerboseLogging());
	} else if(code == KeyEvent::KEY_e){
		if(mEngine.isShowingSettingsEditor()){
			mEngine.hideSettingsEditor();
		} else {
			mEngine.showSettingsEditor(mEngineSettings);
		}
	} else if(ci::app::KeyEvent::KEY_p == code){
		mEngine.getPangoFontService().logFonts(e.isShiftDown());
	} else if(code == ci::app::KeyEvent::KEY_d && e.isControlDown()){
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
	} else if(mArrowKeyCameraControl && code == ci::app::KeyEvent::KEY_LEFT) {
		mEngineData.mSrcRect.x1 -= mArrowKeyCameraStep;
		mEngineData.mSrcRect.x2 -= mArrowKeyCameraStep;
		mEngine.markCameraDirty();
	} else if(mArrowKeyCameraControl && code == ci::app::KeyEvent::KEY_RIGHT) {
		mEngineData.mSrcRect.x1 += mArrowKeyCameraStep;
		mEngineData.mSrcRect.x2 += mArrowKeyCameraStep;
		mEngine.markCameraDirty();
	} else if(mArrowKeyCameraControl && code == ci::app::KeyEvent::KEY_UP) {
		mEngineData.mSrcRect.y1 -= mArrowKeyCameraStep;
		mEngineData.mSrcRect.y2 -= mArrowKeyCameraStep;
		mEngine.markCameraDirty();
	} else if(mArrowKeyCameraControl && code == ci::app::KeyEvent::KEY_DOWN) {
		mEngineData.mSrcRect.y1 += mArrowKeyCameraStep;
		mEngineData.mSrcRect.y2 += mArrowKeyCameraStep;
		mEngine.markCameraDirty();
	} else if(e.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		/// TODO: reload engine settings	
		resetupServer();
	} else if(e.getCode() == KeyEvent::KEY_d){
		const size_t numRoots = mEngine.getRootCount();
		int numPlacemats = 0;
		for(size_t i = 0; i < numRoots - 1; i++){
			mEngine.getRootSprite(i).forEachChild([this](ds::ui::Sprite& sprite){
				if(sprite.isEnabled()){
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
	} else if(e.getCode() == KeyEvent::KEY_f) {
		setFullScreen(!isFullScreen());
	} else if(e.getCode() == KeyEvent::KEY_a){
		ci::app::getWindow()->setAlwaysOnTop(!ci::app::getWindow()->isAlwaysOnTop());
	} else if(e.getCode() == ci::app::KeyEvent::KEY_i) {
		if(mEngine.isIdling()) {
			mEngine.resetIdleTimeout();
		} else {
			mEngine.startIdling();
		}
	} else {
		onKeyDown(e);
	}
	
}

void App::keyUp(ci::app::KeyEvent event){
	if(event.getCode() == ci::app::KeyEvent::KEY_LCTRL || event.getCode() == ci::app::KeyEvent::KEY_RCTRL){
		mCtrlDown = false;
	}

	onKeyUp(event);
}

void App::saveTransparentScreenshot(){
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
