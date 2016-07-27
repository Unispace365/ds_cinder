#include "private/web_service.h"

#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/web.h>

#include "include/cef_app.h"
#include "web_handler.h"

namespace ds {
namespace web {

/**
 * \class ds::web::Service
 */
WebCefService::WebCefService(ds::Engine& e)
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

WebCefService::~WebCefService() {

#ifdef _DEBUG
	std::this_thread::sleep_for(std::chrono::seconds(1));
#endif
	try{
		CefShutdown();
	} catch(...){
		DS_LOG_WARNING("WebCefService destructor exception");
	}
}

void WebCefService::start() {

	CefEnableHighDPISupport();

	void* sandbox_info = NULL;
	CefMainArgs main_args(GetModuleHandle(NULL));

	int exit_code = CefExecuteProcess(main_args, NULL, sandbox_info);
	if(exit_code >= 0){
		DS_LOG_WARNING("CEF setup exit code is not the expected value! Code: " << exit_code);
	}

	CefSettings settings;
	// There's no sandboxing for IO requests
	settings.no_sandbox = true;

	// Single process could be used for debugging, but it's much slower and not-production ready
	settings.single_process = false;
	
	// CEF handles threading for the message loop (required for performance, otherwise slow apps can become deadlocked)
	settings.multi_threaded_message_loop = true;

	// We're using Offscreen Rendering.
	settings.windowless_rendering_enabled = true;

	// CEF's multi-process structure: it's required.
	// There's basically another process that needs to be spawned for different things (rendering, plugins, IO, etc)
	// So we create a small exe that's just there to run CEF. 
	// It's based on cefsimple, from the CEF binary distribution, in case you need to recompile (just remove most of the stuff in cefsimple_win.cc) and it's subsystem to console
	// Here's the entirety of the source for that app, which is setup as a console app:
	/*
	#include <include/cef_app.h>
	int main(int argc, char* argv[]){
		CefMainArgs main_args;
		return CefExecuteProcess(main_args, NULL, NULL);
	}
	*/

	// This requires cefsimple.exe to be in the current working directory
	const char* path = "cefsimple.exe";
	CefString(&settings.browser_subprocess_path).FromASCII(path);

	mCefSimpleApp = CefRefPtr<WebApp>(new WebApp);
	CefInitialize(main_args, settings, mCefSimpleApp.get(), sandbox_info);
}

void WebCefService::update(const ds::UpdateParams&) {
	//CefDoMessageLoopWork();
}

void WebCefService::createBrowser(const std::string& startUrl, void * instancePtr, std::function<void(int)> createdCallback, const bool isTransparent){
	if(mCefSimpleApp){
		try{
			mCefSimpleApp->createBrowser(startUrl, instancePtr, createdCallback, isTransparent);
		} catch(std::exception& e){
			DS_LOG_WARNING("WebCefService: Exception creating browser: " << e.what());
		}
	}
}

void WebCefService::cancelCreation(void * instancePtr){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->cancelCreation(instancePtr);
	}
}

void WebCefService::closeBrowser(const int browserId){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->closeBrowser(browserId);
	}
}

void WebCefService::addWebCallbacks(int browserId, WebCefCallbacks& callbacks){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->addWebCallbacks(browserId, callbacks);
	}
}

void WebCefService::sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state, const int clickCount){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->sendMouseClick(browserId, x, y, bttn, state, clickCount);
	}
}

void WebCefService::sendMouseWheelEvent(const int browserId, const int x, const int y, const int xDelta, const int yDelta){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->sendMouseWheelEvent(browserId, x, y, xDelta, yDelta);
	}
}

void WebCefService::sendKeyEvent(const int browserId, const int state, int windows_key_code, char character, const bool shiftDown, const bool cntrlDown, const bool altDown){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->sendKeyEvent(browserId, state, windows_key_code, character, shiftDown, cntrlDown, cntrlDown);
	}
}

void WebCefService::loadUrl(const int browserId, const std::string& newUrl){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->loadUrl(browserId, newUrl);
	}

}

void WebCefService::requestBrowserResize(const int browserId, const ci::Vec2i newSize){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->requestBrowserResize(browserId, newSize);
	}
}

void WebCefService::goForwards(const int browserId){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->goForwards(browserId);
	}
}

void WebCefService::goBackwards(const int browserId){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->goBackwards(browserId);
	}
}

void WebCefService::reload(const int browserId, const bool ignoreCache){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->reload(browserId, ignoreCache);
	}
}

void WebCefService::stopLoading(const int browserId){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->stopLoading(browserId);
	}
}

void WebCefService::setZoomLevel(const int browserId, const double newZoomLevel){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		handler->setZoomLevel(browserId, newZoomLevel);
	}
}

double WebCefService::getZoomLevel(const int browserId){
	CefRefPtr<WebHandler> handler(WebHandler::GetInstance());
	if(handler){
		return handler->getZoomLevel(browserId);
	}

	return 0.0;
}

} // namespace web
} // namespace ds