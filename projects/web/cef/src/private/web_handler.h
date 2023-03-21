// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef DS_WEB_PRIVATE_CEF_WEB_HANDLER
#define DS_WEB_PRIVATE_CEF_WEB_HANDLER

/// This class has been modified from the CEF example SimpleApp's SimpleHandler

#include "include/base/cef_lock.h"
#include "include/cef_client.h"

#include <cinder/Vector.h>
#include <functional>
#include <list>

#include "web_callbacks.h"

namespace ds { namespace web {


	class WebHandler : public CefClient,
					   public CefDisplayHandler,
					   public CefLifeSpanHandler,
					   public CefLoadHandler,
					   public CefRenderHandler,
					   public CefJSDialogHandler,
					   public CefFocusHandler,
					   public CefKeyboardHandler,
					   public CefRequestHandler,
					   public CefContextMenuHandler {
	  public:
		explicit WebHandler();
		~WebHandler();

		// Provide access to the single global instance of this object.
		static WebHandler* GetInstance();

		// CefClient methods:
		virtual CefRefPtr<CefDisplayHandler>	 GetDisplayHandler() override { return this; }
		virtual CefRefPtr<CefLifeSpanHandler>	 GetLifeSpanHandler() override { return this; }
		virtual CefRefPtr<CefLoadHandler>		 GetLoadHandler() override { return this; }
		virtual CefRefPtr<CefRenderHandler>		 GetRenderHandler() override { return this; }
		virtual CefRefPtr<CefJSDialogHandler>	 GetJSDialogHandler() override { return this; }
		virtual CefRefPtr<CefFocusHandler>		 GetFocusHandler() override { return this; }
		virtual CefRefPtr<CefKeyboardHandler>	 GetKeyboardHandler() override { return this; }
		virtual CefRefPtr<CefRequestHandler>	 GetRequestHandler() override { return this; }
		virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override { return this; }

		// CefDisplayHandler methods:
		virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;


		virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level, const CefString& message,
									  const CefString& source, int line) override;

		virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen) override;

		// CefLifeSpanHandler methods:
		virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
		virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
		virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

		virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
								   const CefString& target_url, const CefString& target_frame_name,
								   CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture,
								   const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
								   CefRefPtr<CefClient>& client, CefBrowserSettings& settings,
								   CefRefPtr<CefDictionaryValue>& extra_info, bool* no_javascript_access) override;

		// CefLoadHandler methods:
		virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
								 const CefString& errorText, const CefString& failedUrl) override;

		virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack,
										  bool canGoForward) override;

		// CefRenderHandler methods:
		virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
		virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX,
									int& screenY) override;
		virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;
		virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

		virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
							 const void* buffer, int width, int height) override;

		virtual bool OnCursorChange(CefRefPtr<CefBrowser> browser, HCURSOR cursor, cef_cursor_type_t type,
									const CefCursorInfo& custom_cursor_info) override;

		virtual bool StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data,
								   DragOperationsMask allowed_ops, int x, int y) override {
			return true;
		}


		virtual void OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser, TextInputMode input_mode);

		virtual void OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value) override;
		virtual bool OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source) override;
		virtual void OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) override;

		virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event,
								CefEventHandle os_event) override;

		// CefContextMenuHandler methods
		void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
								 CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;
		bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
								  CefRefPtr<CefContextMenuParams> params, int command_id,
								  EventFlags event_flags) override;

		// CefJSDialogHandler
		// TODO: Callback to UI and optionally handle
		virtual bool OnJSDialog(CefRefPtr<CefBrowser> browser, const CefString& origin_url, JSDialogType dialog_type,
								const CefString& message_text, const CefString& default_prompt_text,
								CefRefPtr<CefJSDialogCallback> callback, bool& suppress_message) {
			suppress_message = true;
			DS_LOG_INFO("JS Dialog: " << message_text.ToString() << " from: " << origin_url.ToString());
			return false;
		}

		virtual bool GetAuthCredentials(CefRefPtr<CefBrowser> browser, const CefString& origin_url, bool isProxy,
										const CefString& host, int port, const CefString& realm,
										const CefString& scheme, CefRefPtr<CefAuthCallback> callback) override;

		void authRequestCancel(const int browserId);
		void authRequestContinue(const int browserId, const std::string& username, const std::string& password);

		// Requests the browser to be closed and also clears and related callbacks
		void closeBrowser(const int browserId);

		// Adds a callback to a list of callbacks for after browsers are created
		void addCreatedCallback(void* instancePtr, std::function<void(int)> callback);
		void cancelCreation(void* instancePtr);

		// See WebCefCallbacks for info
		void addWebCallbacks(const int browserId, ds::web::WebCefCallbacks& callback);

		// Sends some mouse input to the browser
		void sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state,
							const int clickCount);

		// Sends some mouse wheel input to the browser
		void sendMouseWheelEvent(const int browserId, const int x, const int y, const int xDelta, const int yDelta);

		// Sends a key event to the browser
		void sendKeyEvent(const int browserId, const int state, int windows_key_code, char character,
						  const bool shiftDown, const bool cntrlDown, const bool altDown, const bool isCharacter);

		// Sends a touch event
		void sendTouchEvent(const int browserId, const int touchId, const int x, const int y, const int phase);

		// Loads a new URL in the specified browser's main frame
		void loadUrl(const int browserId, const std::string& newUrl);

		// Execute JavaScript on this browser
		void executeJavascript(const int browserId, const std::string& theJS, const std::string& sourceUrl);

		// Resize the browser. Happens asynchronously, meaning a paint callback will come back later with the actual
		// info
		void requestBrowserResize(const int browserId, const ci::ivec2 newSize);

		// Regular browser controls
		void goForwards(const int browserId);
		void goBackwards(const int browserId);
		void reload(const int browserId, const bool ignoreCache);
		void stopLoading(const int browserId);

		// Zoom level here is the same as CEF expects: 0.0 = 100% zoom
		void   setZoomLevel(const int browserId, const double newZoom);
		double getZoomLevel(const int browserId);

		// Orphans can be created if the visible instance is released before the browser is created.
		// We keep those browsers around to be used the next time a browser is requested
		bool hasOrphans() { return !mOrphanedBrowsers.empty(); }
		void useOrphan(std::function<void(int)> callback, const std::string startUrl);

		// No browser ID cause cookies are global
		// Url and cookie name are optional. Will delete all if not specified.
		void deleteCookies(const std::string& url, const std::string& cookieName);

	  private:
		// switch to a map for faster lookup
		// List of existing browser windows. Only accessed on the CEF UI thread.
		std::map<int, CefRefPtr<CefBrowser>> mBrowserList;

		// Track the sizes that we want the browsers to be, since resizing is asynchronous
		std::map<int, ci::ivec2> mBrowserSizes;

		std::map<void*, std::function<void(int)>> mCreatedCallbacks;
		std::map<int, ds::web::WebCefCallbacks>	  mWebCallbacks;

		std::map<int, CefRefPtr<CefAuthCallback>> mAuthCallbacks;

		// Browsers that were created but their instances were removed before they were used
		std::vector<CefRefPtr<CefBrowser>> mOrphanedBrowsers;

		// The lock is used to ensure stuff that is immediately returned to the main app thread
		// is synchronized with the rest of CEF
		base::Lock mLock;

		// Include the default reference counting implementation.
		IMPLEMENT_REFCOUNTING(WebHandler);
	};

}} // namespace ds::web

#endif // DS_WEB_PRIVATE_CEF_WEB_HANDLER
