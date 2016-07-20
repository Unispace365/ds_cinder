// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

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

	SimpleHandler* g_instance = NULL;

}  // namespace

SimpleHandler::SimpleHandler()
: is_closing_(false) {
	DCHECK(!g_instance);
	g_instance = this;
}

SimpleHandler::~SimpleHandler() {
	g_instance = NULL;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
	return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
								  const CefString& title) {
	CEF_REQUIRE_UI_THREAD();

	// Set the title of the window using platform APIs.
	//PlatformTitleChange(browser, title);
	
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
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

	std::cout << "On after created " << browser->GetIdentifier() << " " << std::this_thread::get_id() << std::endl;

	if(!mCreatedCallbacks.empty()){
		mCreatedCallbacks.front()(browser->GetIdentifier());
		mCreatedCallbacks.erase(mCreatedCallbacks.begin());
	}
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	std::cout << "Simple Handler DoClose" << std::endl;

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed destription of this
	// process.
	if(mBrowserList.size() == 1) {
		// Set a flag to indicate that the window close should be allowed.
		is_closing_ = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}


void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
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

	if(mBrowserList.empty()) {
		// All browser windows have closed. Quit the application message loop.
		//CefQuitMessageLoop();
	}
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
								CefRefPtr<CefFrame> frame,
								ErrorCode errorCode,
								const CefString& errorText,
								const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();

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


void SimpleHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward){

	std::cout << "OnLoadingStateChange: " << isLoading << " "  << std::this_thread::get_id() << std::endl;
}

bool SimpleHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect){
	return false;
}

bool SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect){
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

bool SimpleHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY){
	screenX = viewX;
	screenY = viewY;
	return true;
}

void SimpleHandler::OnPaint(CefRefPtr<CefBrowser> browser,
							PaintElementType type, 
							const RectList& dirtyRects, 
							const void* buffer, int width, int height){
	//CEF_REQUIRE_UI_THREAD();

	int browserId = browser->GetIdentifier();
	auto paintCallback = mPaintCallbacks.find(browserId);
	if(paintCallback != mPaintCallbacks.end() && paintCallback->second){
		paintCallback->second(buffer, width, height);
	}
}

void SimpleHandler::CloseBrowser(const int browserId){

	auto findyS = mBrowserSizes.find(browserId);
	if(findyS != mBrowserSizes.end()){
		mBrowserSizes.erase(findyS);
	}

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		auto browserHost = findy->second->GetHost();
		mBrowserList.erase(findy);
		browserHost->CloseBrowser(true);
	}
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {

	if(mBrowserList.empty())
		return;

	// This gets called from distinctly NOT the UI thread, but not quite sure if that's required.
	// The documentation seems to indicate that it can be called from any thread, as long as it's the browser process (which it is)
	//CEF_REQUIRE_UI_THREAD();
	/*
	if(!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI,
					base::Bind(&SimpleHandler::CloseAllBrowsers, this, force_close));
		return;
	}
	*/


	static bool closedAllAlready = false;
	std::cout << "close all browsers start " << closedAllAlready << std::endl;
	if(closedAllAlready) return;
	closedAllAlready = true;

	mPaintCallbacks.clear();


	std::cout << "close all browsers, num browsers: " << mBrowserList.size() << std::endl;

	for(auto it = mBrowserList.begin(); mBrowserList.size() > 0 && it != mBrowserList.end(); it++){
		if(mBrowserList.size() < 1) return;
		it->second->GetHost()->CloseBrowser(force_close);
	}
}

void SimpleHandler::addCreatedCallback(std::function<void(int)> callback){
	mCreatedCallbacks.push_back(callback);
}

void SimpleHandler::addPaintCallback(int browserId, std::function<void(const void *, const int, const int)> callback){
	mPaintCallbacks[browserId] = callback;
}

void SimpleHandler::sendMouseClick(const int browserId, const int x, const int y, const int bttn, const int state, const int clickCount){
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

// TODO: Move these parsers to a different file
bool IsKeyDown(WPARAM wparam) {
	return (GetKeyState(wparam) & 0x8000) != 0;
}

int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam) {

	int modifiers = 0;
	if(IsKeyDown(VK_SHIFT))
		modifiers |= EVENTFLAG_SHIFT_DOWN;
	if(IsKeyDown(VK_CONTROL))
		modifiers |= EVENTFLAG_CONTROL_DOWN;
	if(IsKeyDown(VK_MENU))
		modifiers |= EVENTFLAG_ALT_DOWN;

	// Low bit set from GetKeyState indicates "toggled".
	if(::GetKeyState(VK_NUMLOCK) & 1)
		modifiers |= EVENTFLAG_NUM_LOCK_ON;
	if(::GetKeyState(VK_CAPITAL) & 1)
		modifiers |= EVENTFLAG_CAPS_LOCK_ON;

	switch(wparam) {
	case VK_RETURN:
		if((lparam >> 16) & KF_EXTENDED)
			modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
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
		if(!((lparam >> 16) & KF_EXTENDED))
			modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
	case VK_NUMLOCK:
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
	case VK_DIVIDE:
	case VK_MULTIPLY:
	case VK_SUBTRACT:
	case VK_ADD:
	case VK_DECIMAL:
	case VK_CLEAR:
		modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
	case VK_SHIFT:
		if(IsKeyDown(VK_LSHIFT))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if(IsKeyDown(VK_RSHIFT))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_CONTROL:
		if(IsKeyDown(VK_LCONTROL))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if(IsKeyDown(VK_RCONTROL))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_MENU:
		if(IsKeyDown(VK_LMENU))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if(IsKeyDown(VK_RMENU))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_LWIN:
		modifiers |= EVENTFLAG_IS_LEFT;
		break;
	case VK_RWIN:
		modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	}
	return modifiers;
}

void SimpleHandler::sendKeyEvent(const int browserId, const int state, int windows_key_code, int native_key_code, unsigned int modifiers, char character){
	CefKeyEvent keyEvent;
	keyEvent.type = KEYEVENT_RAWKEYDOWN;
	keyEvent.windows_key_code = 65;	
	keyEvent.native_key_code = 1966081;// (int)(OemKeyScan(character));
	keyEvent.modifiers = 0;// GetCefKeyboardModifiers(native_key_code, 0);
	if(modifiers & 0x0008){
	//	keyEvent.modifiers |= EVENTFLAG_SHIFT_DOWN;
	}

	/*
	CefKeyEvent event;
	event.windows_key_code = wParam;
	event.native_key_code = lParam;
	event.is_system_key = message == WM_SYSCHAR ||
		message == WM_SYSKEYDOWN ||
		message == WM_SYSKEYUP;

	if(message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
		event.type = KEYEVENT_RAWKEYDOWN;
	else if(message == WM_KEYUP || message == WM_SYSKEYUP)
		event.type = KEYEVENT_KEYUP;
	else
		event.type = KEYEVENT_CHAR;
	event.modifiers = GetCefKeyboardModifiers(wParam, lParam);
	*/

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){

		auto browserHost = findy->second->GetHost();

		if(state == 0){
			CefKeyEvent keyEvent;
			keyEvent.type = KEYEVENT_RAWKEYDOWN;
			keyEvent.windows_key_code = 65;
			keyEvent.native_key_code = 1966081;// (int)(OemKeyScan(character));
			keyEvent.modifiers = 0;// GetCefKeyboardModifiers(native_key_code, 0);
			browserHost->SendKeyEvent(keyEvent);

		} else {

			int scanCode = MapVirtualKey(character, 0);

			CefKeyEvent keyEvent;
			keyEvent.type = KEYEVENT_CHAR;
			keyEvent.windows_key_code = character;
			keyEvent.native_key_code = scanCode;// (int)(OemKeyScan(character));
			keyEvent.modifiers = 0;// GetCefKeyboardModifiers(native_key_code, 0);
			browserHost->SendKeyEvent(keyEvent);
		}
	} else {
		DS_LOG_WARNING("Couldn't find the correct browser to send key event to! BrowserId = " << browserId);
	}
}

void SimpleHandler::loadUrl(const int browserId, const std::string& newUrl){

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		findy->second->GetMainFrame()->LoadURL(CefString(newUrl));
	}
}

void SimpleHandler::requestBrowserResize(const int browserId, const ci::Vec2i newSize){

	mBrowserSizes[browserId] = newSize;

	auto findy = mBrowserList.find(browserId);
	if(findy != mBrowserList.end()){
		// This tells the browser to get the view size, in which case it will look up the browser size in the mBrowserSizes map and resize accordingly, then repaint
		findy->second->GetHost()->WasResized();
	}

}
