// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"

#include <functional>
#include <list>
#include <cinder/Vector.h>

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
	virtual bool GetRootScreenRect(CefRefPtr<CefBrowser> browser,
								   CefRect& rect);
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect);
	virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser,
								int viewX,
								int viewY,
								int& screenX,
								int& screenY);

	virtual void OnPaint(CefRefPtr<CefBrowser> browser,
						 PaintElementType type,
						 const RectList& dirtyRects,
						 const void* buffer,
						 int width, int height) OVERRIDE;

	void CloseBrowser(const int browserId);

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);

	bool IsClosing() const { return is_closing_; }

	// Adds a callback to a list of callbacks for after browsers are created
	void addCreatedCallback(std::function<void(int)> callback);

	// Gets called when the browser sends new paint info, aka new buffers
	void addPaintCallback(int browserId, std::function<void(const void *, const int bufferWidth, const int bufferHeight)> callback);

	// Sends some mouse input to the browser
	void sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state, const int clickCount);

	// Sends a key event to the browser
	void sendKeyEvent(const int browserId, const int state, int windows_key_code, int native_key_code, unsigned int modifiers, char character);

	// Loads a new URL in the specified browser's main frame
	void loadUrl(const int browserId, const std::string& newUrl);

	// Resize the browser. Happens asynchronously, meaning a paint callback will come back later with the actual info
	void requestBrowserResize(const int browserId, const ci::Vec2i newSize);

private:

	// switch to a map for faster lookup
	// List of existing browser windows. Only accessed on the CEF UI thread.
	std::map<int, CefRefPtr<CefBrowser>>					mBrowserList;

	// Track the sizes that we want the browsers to be, since resizing is asynchronous
	std::map<int, ci::Vec2i>								mBrowserSizes;

	bool is_closing_;

	std::vector<std::function<void(int)>>					mCreatedCallbacks;
	std::map<int, std::function<void(const void *, const int, const int)>> mPaintCallbacks;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
