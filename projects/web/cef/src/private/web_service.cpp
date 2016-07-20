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

	//CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	//if(handler){
	//	handler->CloseAllBrowsers(false);
//	}

	//std::this_thread::sleep_for(std::chrono::seconds(2));

	try{
		CefShutdown();
	} catch(...){
		std::cout << "Service destructor exception" << std::endl;
	}
}

void Service::start() {

	CefEnableHighDPISupport();

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
	settings.single_process = false;
	settings.multi_threaded_message_loop = false;
	settings.windowless_rendering_enabled = false;

	//const char* path = "D:/code/cef_binary_3.2704.1431.ge7ddb8a_windows32/cefsimple/Release/cefsimple.exe";
	//const char* path = ds::Environment::expand("%APP%/cefsimple.exe").c_str();
	const char* path = "cefsimple.exe";
	CefString(&settings.browser_subprocess_path).FromASCII(path);

	mCefSimpleApp = CefRefPtr<SimpleApp>(new SimpleApp);
	CefInitialize(main_args, settings, mCefSimpleApp.get(), sandbox_info);
	std::cout << "cef initialize" << std::endl;
}

void Service::update(const ds::UpdateParams&) {
	CefDoMessageLoopWork();
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

void Service::closeBrowser(const int browserId){
	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->CloseBrowser(browserId);
	}
}

void Service::addPaintCallback(int browserId, std::function<void(const void *, const int, const int)> paintCallback){
	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->addPaintCallback(browserId, paintCallback);
	}
}

void Service::addLoadChangeCallback(const int browserId, std::function<void(const bool isLoading, const bool canBack, const bool canForward)> callback){
	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->addLoadChangeCallback(browserId, callback);
	}
}

void Service::addTitleChangeCallback(const int browserId, std::function<void(const std::wstring&)> callback){
	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->addTitleChangeCallback(browserId, callback);
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

void Service::loadUrl(const int browserId, const std::string& newUrl){
	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->loadUrl(browserId, newUrl);
	}

}

void Service::requestBrowserResize(const int browserId, const ci::Vec2i newSize){
	CefRefPtr<SimpleHandler> handler(SimpleHandler::GetInstance());
	if(handler){
		handler->requestBrowserResize(browserId, newSize);
	}
}

} // namespace web
} // namespace ds