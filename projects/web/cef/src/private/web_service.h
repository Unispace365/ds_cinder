#pragma once
#ifndef PRIVATE_CEF_WEBSERVICE_H_
#define PRIVATE_CEF_WEBSERVICE_H_

#include <ds/app/engine/engine_service.h>
#include <ds/app/auto_update.h>

#include "web_callbacks.h"
#include "simple_app.h"

namespace ds {
class Engine;

namespace web {

/**
 * \class ds::web::Service
 * \brief The engine service object that provides access to the Chromium Embedded Framework objects.
 *			This class initializes CEF and provides some high-level functions.
 *			Then it acts as a pass-through to the underlying functionality, which is mostly in the handler.
 */
class Service : public ds::EngineService,
				public ds::AutoUpdate {
public:
	Service(ds::Engine&);
	~Service();

	virtual void			start();

	void					createBrowser(const std::string& startUrl, std::function<void(int)> browserCreatedCallback);
	void					closeBrowser(const int browserId);

	void					addWebCallbacks(const int browserId, WebCefCallbacks& callback);

	// browser id was returned from the createBrowser callback, x / y in pixels from top/left in browser space, bttn: left=0, middle = 1; right = 2, state: 0 = down, 1 = move, 2 = release. 
	void					sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state, const int clickCount);

	void					sendKeyEvent(const int browserId, const int state, int windows_key_code, int native_key_code, unsigned int modifiers, char character);

	void					loadUrl(const int browserId, const std::string& newUrl);

	void					requestBrowserResize(const int browserId, const ci::Vec2i newSize);

protected:
	virtual void			update(const ds::UpdateParams&);

private:
	CefRefPtr<SimpleApp>	mCefSimpleApp;
};

} // namespace web
} // namespace ds

#endif // PRIVATE_WEBSERVICE_H_