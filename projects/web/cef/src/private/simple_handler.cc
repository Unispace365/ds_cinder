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

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser,
										const CefString& title) {
	CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
	SetWindowText(hwnd, std::wstring(title).c_str());
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Add to the list of existing browsers.
	browser_list_.push_back(browser);
	std::cout << "On after created " << browser->GetIdentifier() << " " << std::this_thread::get_id() << std::endl;

	if(!mCreatedCallbacks.empty()){
		mCreatedCallbacks.front()(browser->GetIdentifier());
		mCreatedCallbacks.erase(mCreatedCallbacks.begin());
	}
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed destription of this
	// process.
	if(browser_list_.size() == 1) {
		// Set a flag to indicate that the window close should be allowed.
		is_closing_ = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}


void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Remove from the list of existing browsers.
	BrowserList::iterator bit = browser_list_.begin();
	for(; bit != browser_list_.end(); ++bit) {
		if((*bit)->IsSame(browser)) {
			browser_list_.erase(bit);
			break;
		}
	}

	if(browser_list_.empty()) {
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


bool SimpleHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect){
	rect.x = rect.y = 0;
	rect.width = 1920;
	rect.height = 1080;
	return true;
}

bool SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect){
	//TODO
	rect.x = rect.y = 0;
	rect.width = 1920; 
	rect.height = 1080; 
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
	std::cout << "On Paint: " << width << " " << height << std::endl;
	//CEF_REQUIRE_UI_THREAD();

	int browserId = browser->GetIdentifier();
	auto paintCallback = mPaintCallbacks.find(browserId);
	if(paintCallback != mPaintCallbacks.end() && paintCallback->second){
		paintCallback->second(buffer);
	}
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {

	static bool closedAllAlready = false;
	if(closedAllAlready) return;
	closedAllAlready = true;
	std::cout << "Close all browsers " << force_close << std::endl;
	if(!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI,
					base::Bind(&SimpleHandler::CloseAllBrowsers, this, force_close));
		return;
	}

	mPaintCallbacks.clear();

	if(browser_list_.empty())
		return;

	BrowserList::const_iterator it = browser_list_.begin();
	for(;browser_list_.size() > 0 && it != browser_list_.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);
}

void SimpleHandler::addCreatedCallback(std::function<void(int)> callback){
	mCreatedCallbacks.push_back(callback);
}

void SimpleHandler::addPaintCallback(int browserId, std::function<void(const void *)> callback){
	mPaintCallbacks[browserId] = callback;
}
