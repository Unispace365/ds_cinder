// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"

#include <functional>
#include <list>
#include <cinder/Vector.h>

#include "web_callbacks.h"

class SimpleHandler : public CefClient,
	public CefDisplayHandler,
	public CefLifeSpanHandler,
	public CefLoadHandler,
	public CefRenderHandler
{
public:
	explicit SimpleHandler();
	~SimpleHandler();

	// Provide access to the single global instance of this object.
	static SimpleHandler* GetInstance();

	// CefClient methods:
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE{
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE{
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE{
		return this;
	}
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE{
		return this;
	}

	// CefDisplayHandler methods:
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
							   const CefString& title) OVERRIDE;

	// CefLifeSpanHandler methods:
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	// Called on the IO thread before a new popup browser is created. The
	// |browser| and |frame| values represent the source of the popup request. The
	// |target_url| and |target_frame_name| values indicate where the popup
	// browser should navigate and may be empty if not specified with the request.
	// The |target_disposition| value indicates where the user intended to open
	// the popup (e.g. current tab, new tab, etc). The |user_gesture| value will
	// be true if the popup was opened via explicit user gesture (e.g. clicking a
	// link) or false if the popup opened automatically (e.g. via the
	// DomContentLoaded event). The |popupFeatures| structure contains additional
	// information about the requested popup window. To allow creation of the
	// popup browser optionally modify |windowInfo|, |client|, |settings| and
	// |no_javascript_access| and return false. To cancel creation of the popup
	// browser return true. The |client| and |settings| values will default to the
	// source browser's values. If the |no_javascript_access| value is set to
	// false the new browser will not be scriptable and may not be hosted in the
	// same renderer process as the source browser. Any modifications to
	// |windowInfo| will be ignored if the parent browser is wrapped in a
	// CefBrowserView.
	///
	/*--cef(optional_param=target_url,optional_param=target_frame_name)--*/
	virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
							   CefRefPtr<CefFrame> frame,
							   const CefString& target_url,
							   const CefString& target_frame_name,
							   WindowOpenDisposition target_disposition,
							   bool user_gesture,
							   const CefPopupFeatures& popupFeatures,
							   CefWindowInfo& windowInfo,
							   CefRefPtr<CefClient>& client,
							   CefBrowserSettings& settings,
							   bool* no_javascript_access);

	// CefLoadHandler methods:
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
							 CefRefPtr<CefFrame> frame,
							 ErrorCode errorCode,
							 const CefString& errorText,
							 const CefString& failedUrl) OVERRIDE;

	virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
									  bool isLoading,
									  bool canGoBack,
									  bool canGoForward) OVERRIDE;

	// CefRenderHandler methods:
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) OVERRIDE;
	virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
								int viewX,
								int viewY,
								int& screenX,
								int& screenY) OVERRIDE;

	virtual void OnPaint(CefRefPtr<CefBrowser> browser,
						 PaintElementType type,
						 const RectList& dirtyRects,
						 const void* buffer,
						 int width, int height) OVERRIDE;

	virtual bool StartDragging(CefRefPtr<CefBrowser> browser,
							   CefRefPtr<CefDragData> drag_data,
							   DragOperationsMask allowed_ops,
							   int x, int y) OVERRIDE {
		return true;
	}

	// Requests the browser to be closed and also clears and related callbacks
	void CloseBrowser(const int browserId);

	// Adds a callback to a list of callbacks for after browsers are created
	void addCreatedCallback(std::function<void(int)> callback);

	void addWebCallbacks(const int browserId, ds::web::WebCefCallbacks& callback);

	// Sends some mouse input to the browser
	void sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state, const int clickCount);

	// Sends a key event to the browser
	void sendKeyEvent(const int browserId, const int state, int windows_key_code, int native_key_code, unsigned int modifiers, char character);

	// Loads a new URL in the specified browser's main frame
	void loadUrl(const int browserId, const std::string& newUrl);

	// Resize the browser. Happens asynchronously, meaning a paint callback will come back later with the actual info
	void requestBrowserResize(const int browserId, const ci::Vec2i newSize);

	// Regular browser controls
	void goForwards(const int browserId);
	void goBackwards(const int browserId);
	void reload(const int browserId, const bool ignoreCache);
	void stopLoading(const int browserId);

private:

	// switch to a map for faster lookup
	// List of existing browser windows. Only accessed on the CEF UI thread.
	std::map<int, CefRefPtr<CefBrowser>>					mBrowserList;

	// Track the sizes that we want the browsers to be, since resizing is asynchronous
	std::map<int, ci::Vec2i>								mBrowserSizes;

	std::vector<std::function<void(int)>>					mCreatedCallbacks;
	std::map<int, ds::web::WebCefCallbacks>					mWebCallbacks;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
