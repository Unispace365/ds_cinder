#pragma once
#ifndef PRIVATE_CEF_WEBSERVICE_H_
#define PRIVATE_CEF_WEBSERVICE_H_

#include <ds/app/engine/engine_service.h>
#include <ds/app/auto_update.h>

#include "web_callbacks.h"
#include "web_app.h"

namespace ds {
class Engine;

namespace web {

/**
 * \class ds::web::Service
 * \brief The engine service object that provides access to the Chromium Embedded Framework objects.
 *			This class initializes CEF and provides some high-level functions.
 *			Then it acts as a pass-through to the underlying functionality, which is mostly in WebHandler.

 *	Overall Structure: Service (this) -> create WebApp (initializes context and creates browsers -> creates WebHandler (a single class that manages all the browser instances)
 *						Each Browser (a CEF concept / construct) is a single unit that loads a url into a main frame (and potentially nested frames). 
 *						WebSprites get one browser each. Since browsers are created asynchronously, there's a callback for the WebSprite to grab the browser id after creation.
 *						Once the browser is created (generally within a frame or two), then subsequent functions can be called (forward/back/reload/etc) 
 *						and setup callbacks (for functions such as title changes, errors, new browsing locations, updated view buffers)
 *						CEF runs multiple processes for each browser instance, which is handled by cefsimple.exe, which must be in the working directory.
 */
class WebCefService : public ds::EngineService,
				public ds::AutoUpdate {
public:
	WebCefService(ds::Engine&);
	~WebCefService();

	virtual void			start();

	void					createBrowser(const std::string& startUrl, std::function<void(int)> browserCreatedCallback);
	void					closeBrowser(const int browserId);

	void					addWebCallbacks(const int browserId, WebCefCallbacks& callback);

	// browser id was returned from the createBrowser callback, x / y in pixels from top/left in browser space, bttn: left=0, middle = 1; right = 2, state: 0 = down, 1 = move, 2 = release. 
	void					sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state, const int clickCount);

	void					sendKeyEvent(const int browserId, const int state, int windows_key_code, int native_key_code, unsigned int modifiers, char character);

	void					loadUrl(const int browserId, const std::string& newUrl);

	void					requestBrowserResize(const int browserId, const ci::Vec2i newSize);

	void					goForwards(const int browserId);
	void					goBackwards(const int browserId);
	void 					reload(const int browserId, const bool ignoreCache);
	void 					stopLoading(const int browserId);

protected:
	virtual void			update(const ds::UpdateParams&);

private:
	CefRefPtr<WebApp>	mCefSimpleApp;
};

} // namespace web
} // namespace ds

#endif // PRIVATE_WEBSERVICE_H_