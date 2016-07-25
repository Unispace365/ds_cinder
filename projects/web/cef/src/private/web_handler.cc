// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "web_handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include <iostream>
#include <thread>

#include <ds/debug/logger.h>

namespace {

	ds::web::WebHandler* g_instance = NULL;

}  // namespace

namespace ds {
namespace web {


WebHandler::WebHandler()
{
	DCHECK(!g_instance);
	g_instance = this;
}

WebHandler::~WebHandler() {
	g_instance = NULL;
}

// static
WebHandler* WebHandler::GetInstance() {
	return g_instance;
}

void WebHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
								  const CefString& title) {
	CEF_REQUIRE_UI_THREAD();

	int browserId = browser->GetIdentifier();
	auto findy = mWebCallbacks.find(browserId);
	if(findy != mWebCallbacks.end()){
		if(findy->second.mTitleChangeCallback){
			findy->second.mTitleChangeCallback(title.ToWString());
		}
	}
}

void WebHandler::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen){
	CEF_REQUIRE_UI_THREAD();
	int browserId = browser->GetIdentifier();
	auto findy = mWebCallbacks.find(browserId);
	if(findy != mWebCallbacks.end()){
		if(findy->second.mFullscreenCallback){
			findy->second.mFullscreenCallback(fullscreen);
		}
	}
}

void WebHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	int browserIdentifier = browser->GetIdentifier();

	auto findy = mBrowserList.find(browserIdentifier);
	if(findy != mBrowserList.end()){
		DS_LOG_WARNING("Multiple browsers tracked with the same identifier! This could lead to issues. Identifier: " << browserIdentifier);

		findy->second->GetHost()->CloseBrowser(true);
		mBrowserList.erase(findy);
	}

	auto findySize = mBrowserSizes.find(browserIdentifier);
	if(findySize != mBrowserSizes.end()){
		mBrowserSizes.erase(findySize);
	}

	mBrowserList[browserIdentifier] = browser;

	if(!mCreatedCallbacks.empty()){
		mCreatedCallbacks.front()(browser->GetIdentifier());
		mCreatedCallbacks.erase(mCreatedCallbacks.begin());
	}
}

bool WebHandler::DoClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Allow the close.
	return false;
}


void WebHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// clear any tracked sizes
	auto findySize = mBrowserSizes.find(browser->GetIdentifier());
	if(findySize != mBrowserSizes.end()){
		mBrowserSizes.erase(findySize);
	}

	// Remove from the list of existing browsers.
	for(auto bit = mBrowserList.begin(); bit != mBrowserList.end(); ++bit) {
		if(bit->second->IsSame(browser)) {
			mBrowserList.erase(bit);
			break;
		}
	}
}

// This callback happens when there's a request for a new window, tab, page, etc. 
// Currently, we're rerouting this all to load the new page in the same browser
// Could possibly change this behaviour to send a callback to the UI / client code and open a new instance
bool WebHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser, 
								  CefRefPtr<CefFrame> frame, 
								  const CefString& target_url, 
								  const CefString& target_frame_name, 
								  WindowOpenDisposition target_disposition, 
								  bool user_gesture, 
								  const CefPopupFeatures& popupFeatures, 
								  CefWindowInfo& windowInfo, 
								  CefRefPtr<CefClient>& client, 
								  CefBrowserSettings& settings, 
								  bool* no_javascript_access){
	browser->GetMainFrame()->LoadURL(target_url);
	return true; // true prevents the popup
}

void WebHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								ErrorCode errorCode,
								const CefString& errorText,
								const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();


	int browserId = browser->GetIdentifier();
	auto findy = mWebCallbacks.find(browserId);
	if(findy != mWebCallbacks.end()){
		if(findy->second.mErrorCallback){
			std::stringstream ss;
			ss << "Failed to load URL with error " << errorText.ToString() << " number= " << errorCode;
			findy->second.mErrorCallback(ss.str());
		}
	}

	// Don't display an error for downloaded files.
	if(errorCode == ERR_ABORTED)
		return;

	// Display a load error message.
	std::stringstream ss;
	ss << "<html><body bgcolor=\"white\">"
		"<h2>Failed to load URL " << std::string(failedUrl) <<
		" with error " << std::string(errorText) << " (" << errorCode <<
		").</h2></body></html>";
	frame->LoadString(ss.str(), failedUrl);
}


void WebHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward){
	int browserId = browser->GetIdentifier();
	auto findy = mWebCallbacks.find(browserId);
	if(findy != mWebCallbacks.end()){
		if(findy->second.mLoadChangeCallback){
			findy->second.mLoadChangeCallback(isLoading, canGoBack, canGoForward);
		}
	}
}

bool WebHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect){
	rect.x = rect.y = 0;
	auto findy = mBrowserSizes.find(browser->GetIdentifier());
	if(findy == mBrowserSizes.end()){
		// initial size before the main app knows about this browser.
		rect.width = 640;
		rect.height = 480;
	} else {
		// The main app knows about this browser and set the size
		rect.width = findy->second.x;
		rect.height = findy->second.y;
	}
	return true;
}

bool WebHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY){
	screenX = viewX;
	screenY = viewY;
	return true;
}

void WebHandler::OnPaint(CefRefPtr<CefBrowser> browser,
							PaintElementType type, 
							const RectList& dirtyRects, 
							const void* buffer, int width, int height){
	CEF_REQUIRE_UI_THREAD();

	//TODO: Handle dirty rects
	//std::cout << "OnPaint, type: " << type <<  " " << width << " " << height << std::endl;

	int browserId = browser->GetIdentifier();
	auto findy = mWebCallbacks.find(browserId);
	if(findy != mWebCallbacks.end()){
		if(findy->second.mPaintCallback){
			findy->second.mPaintCallback(buffer, width, height);
		}
	}
}

void WebHandler::CloseBrowser(const int browserId){

	auto findyS = mBrowserSizes.find(browserId);
	if(findyS != mBrowserSizes.end()){
		mBrowserSizes.erase(findyS);
	}

	auto findyP = mWebCallbacks.find(browserId);
	if(findyP != mWebCallbacks.end()){
		mWebCallbacks.erase(findyP);
	}

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		auto browserHost = findy->second->GetHost();
		mBrowserList.erase(findy);
		browserHost->CloseBrowser(true);
	}
}

void WebHandler::addCreatedCallback(std::function<void(int)> callback){
	mCreatedCallbacks.push_back(callback);
}

void WebHandler::addWebCallbacks(int browserId, ds::web::WebCefCallbacks& callbacks){
	mWebCallbacks[browserId] = callbacks;
}

void WebHandler::sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state, const int clickCount){
	if(mBrowserList.empty()) return;

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		CefMouseEvent mouseEvent;
		mouseEvent.x = x;
		mouseEvent.y = y;

		auto browserHost = findy->second->GetHost();

		CefBrowserHost::MouseButtonType btnType = (bttn == 0 ? MBT_LEFT : (bttn == 2 ? MBT_RIGHT : MBT_MIDDLE));
		if(state == 0){
			browserHost->SendMouseClickEvent(mouseEvent, btnType, false, clickCount);
		} else if(state == 1){
			browserHost->SendMouseMoveEvent(mouseEvent, false);
		} else {
			browserHost->SendMouseClickEvent(mouseEvent, btnType, false, clickCount);
			browserHost->SendMouseClickEvent(mouseEvent, btnType, true, 0);
		}
	} else {
		DS_LOG_WARNING("Browser not found in list to sendMouseClick to! BrowserId=" << browserId);
	}
	
}

void WebHandler::sendMouseWheelEvent(const int browserId, const int x, const int y, const int xDelta, const int yDelta){
	if(mBrowserList.empty()) return;

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		auto browserHost = findy->second->GetHost();
		CefMouseEvent mouseEvent;
		mouseEvent.x = x;
		mouseEvent.y = y;

		browserHost->SendMouseWheelEvent(mouseEvent, xDelta, yDelta);
	}
}

void WebHandler::sendKeyEvent(const int browserId, const int state, int windows_key_code, char character, const bool shiftDown, const bool cntrlDown, const bool altDown){

	CefKeyEvent keyEvent;

	bool isChar = false;
	keyEvent.type = KEYEVENT_RAWKEYDOWN;
	keyEvent.native_key_code = MapVirtualKey(windows_key_code, MAPVK_VK_TO_VSC);

	switch(windows_key_code) {
		// These keys are non-character keys and have different windows_key_codes from the character
		// May need to add more codes to this list, or modify as time goes on
		case VK_INSERT:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_NUMLOCK:
		case VK_CLEAR:
		case VK_SHIFT:
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_CONTROL:
		case VK_MENU:
		case VK_LWIN:
		case VK_RWIN:
		case VK_BACK:
		{
			keyEvent.windows_key_code = windows_key_code;
			break;
		}

		default:
			isChar = true;
			keyEvent.windows_key_code = character;
	}

	keyEvent.modifiers = 0;

	if(shiftDown){
		keyEvent.modifiers |= EVENTFLAG_SHIFT_DOWN;
	}

	if(cntrlDown){
		keyEvent.modifiers |= EVENTFLAG_CONTROL_DOWN;
	}

	if(altDown){
		keyEvent.modifiers |= EVENTFLAG_ALT_DOWN;
	}

	// Assume any keys targetted for the number pad (NUMPAD0-9 and Plus/minus/divide, etc) can be characters
	keyEvent.modifiers |= EVENTFLAG_NUM_LOCK_ON;
	keyEvent.modifiers |= EVENTFLAG_IS_KEY_PAD;


	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){

		auto browserHost = findy->second->GetHost();


		if(state == 0){
			if(isChar){
				keyEvent.type = KEYEVENT_CHAR;
			} else {
				keyEvent.type = KEYEVENT_KEYDOWN;
			}
			browserHost->SendKeyEvent(keyEvent);

		} else {
			keyEvent.type = KEYEVENT_KEYUP;
			
			browserHost->SendKeyEvent(keyEvent);
		}
	} else {
		DS_LOG_WARNING("Couldn't find the correct browser to send key event to! BrowserId = " << browserId);
	}
}

void WebHandler::loadUrl(const int browserId, const std::string& newUrl){

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		findy->second->GetMainFrame()->LoadURL(CefString(newUrl));
	}
}

void WebHandler::requestBrowserResize(const int browserId, const ci::Vec2i newSize){
	mBrowserSizes[browserId] = newSize;

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		// This tells the browser to get the view size, in which case it will look up the browser size in the mBrowserSizes map and resize accordingly, then repaint
		findy->second->GetHost()->WasResized();
	}

}

void WebHandler::goForwards(const int browserId){
	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		findy->second->GoForward();
	}
}

void WebHandler::goBackwards(const int browserId){
	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		findy->second->GoBack();
	}
}

void WebHandler::reload(const int browserId, const bool ignoreCache){
	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		if(ignoreCache){
			findy->second->ReloadIgnoreCache();
		} else {
			findy->second->Reload();
		}
	}
}

void WebHandler::stopLoading(const int browserId){
	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		findy->second->StopLoad();
	}
}

double WebHandler::getZoomLevel(const int browserId){
	CEF_REQUIRE_UI_THREAD();
	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		return findy->second->GetHost()->GetZoomLevel();
	}
	
	return 0.0;
}

void WebHandler::setZoomLevel(const int browserId, const double newZoom){
	CEF_REQUIRE_UI_THREAD();
	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		findy->second->GetHost()->SetZoomLevel(newZoom);
	}
}

}
}