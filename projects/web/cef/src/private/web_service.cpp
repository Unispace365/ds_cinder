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
	settings.multi_threaded_message_loop = true;
	//settings.windowless_rendering_enabled

	mCefSimpleApp = CefRefPtr<SimpleApp>(new SimpleApp);
	CefInitialize(main_args, settings, mCefSimpleApp.get(), sandbox_info);
	std::cout << "cef initialize" << std::endl;
}

void Service::createBrowser(const std::string& startUrl){
	if(mCefSimpleApp){
		std::cout << "Create brwoser " << std::this_thread::get_id() << std::endl;
		mCefSimpleApp->createBrowser(startUrl);
	}
}

void Service::update(const ds::UpdateParams&) {
}

} // namespace web
} // namespace ds