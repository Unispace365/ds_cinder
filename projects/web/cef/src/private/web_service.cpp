#include "private/web_service.h"

#include <ds/app/engine/engine.h>

#include <ds/ui/sprite/web.h>

#include "include/cef_app.h"

#include "simple_app.h"
#include "simple_handler.h"

namespace ds {
namespace web {

/**
 * \class ds::web::Service
 */
Service::Service(ds::Engine& e)
	: ds::AutoUpdate(e, AutoUpdateType::SERVER | AutoUpdateType::CLIENT)
{
	mEngine.registerSpriteImporter("web", [this](ds::ui::SpriteEngine& engine)->ds::ui::Sprite*{
		return new ds::ui::Web(mEngine);
	});

	mEngine.registerSpritePropertySetter("web_url", [this](ds::ui::Sprite& bs, const std::string& theValue, const std::string& fileReferrer){
		ds::ui::Web* webby = dynamic_cast<ds::ui::Web*>(&bs);
		if(!webby){
			DS_LOG_WARNING("Tried to set the web_url of a non-ds::ui::Web sprite. Ignoring!");
			return;
		}

		webby->setUrl(theValue);
	});


}

#include <chrono>
#include <thread>

Service::~Service() {

	std::cout << "Service destructor" << std::endl;

	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->CloseAllBrowsers(true);
	}

	std::this_thread::sleep_for(std::chrono::seconds(2));

	CefShutdown();
}

void Service::start() {
	std::cout << "web service startup" << std::endl;
	void* sandbox_info = NULL;
	CefMainArgs main_args(GetModuleHandle(NULL));
	int exit_code = CefExecuteProcess(main_args, NULL, sandbox_info);
	if(exit_code >= 0){
		std::cout << "CEF setup exit code: " << exit_code << std::endl;
	}
	std::cout << "execute process" << std::endl;

	CefSettings settings;
	settings.no_sandbox = true;
	settings.single_process = true;
	settings.multi_threaded_message_loop = true;
	settings.windowless_rendering_enabled = true;

	mCefSimpleApp = CefRefPtr<SimpleApp>(new SimpleApp);
	CefInitialize(main_args, settings, mCefSimpleApp.get(), sandbox_info);
	std::cout << "cef initialize" << std::endl;
}

void Service::createBrowser(const std::string& startUrl, std::function<void(int)> createdCallback){
	if(mCefSimpleApp){
		std::cout << "Service: Create browser " << std::this_thread::get_id() << std::endl;
		try{
			mCefSimpleApp->createBrowser(startUrl, createdCallback);
		} catch(std::exception& e){
			std::cout << "Exception creating browser: " << e.what() << std::endl;
		}
	}
}

void Service::addPaintCallback(int browserId, std::function<void(const void *)> paintCallback){
	if(mCefSimpleApp){
		mCefSimpleApp->addPaintCallback(browserId, paintCallback);
	}
}

void Service::sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state, const int clickCount){
	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->sendMouseClick(browserId, x, y, bttn, state, clickCount);
	}
}

void Service::sendKeyEvent(const int browserId, const int state, int windows_key_code, int native_key_code, unsigned int modifiers, char character){
	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->sendKeyEvent(browserId, state, windows_key_code, native_key_code, modifiers, character);
	}
}

void Service::update(const ds::UpdateParams&) {
	//CefDoMessageLoopWork();
}

} // namespace web
} // namespace ds