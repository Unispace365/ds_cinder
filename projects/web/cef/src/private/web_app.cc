// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "web_app.h"

#include <string>

#include "web_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

#include <ds/debug/debug_defines.h>

namespace ds{
namespace web{

WebApp::WebApp() {
}

#include <iostream>
void WebApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line){

	// This could prevent a crash in debug when closing (but sometimes doesn't):
	//command_line->AppendSwitch(CefString("disable-extensions"));
	command_line->AppendSwitchWithValue(CefString("enable-system-flash"), CefString("1"));
	command_line->AppendSwitch("disable-gpu");
	command_line->AppendSwitch("disable-gpu-compositing");

	// Some docs online say to use these two, but I had bad results with these
//	command_line->AppendSwitch("disable-surfaces");
//	command_line->AppendSwitch("enable-begin-frame-scheduling");

	command_line->AppendSwitch("off-screen-rendering-enabled");
	command_line->AppendSwitchWithValue("off-screen-frame-rate", "60");
	command_line->AppendSwitchWithValue(CefString("touch-optimized-ui"),CefString("enabled"));
}

void WebApp::OnContextInitialized() {
	CEF_REQUIRE_UI_THREAD();

	/* in case you need to check what's been enabled or disabled
	CefRefPtr<CefCommandLine> command_line =
		CefCommandLine::GetGlobalCommandLine();

	const bool disable_extensions = command_line->HasSwitch("disable-extensions");
	*/

	// SimpleHandler implements browser-level callbacks.
	mHandler = CefRefPtr<WebHandler>(new WebHandler());
}

void WebApp::createBrowser(const std::string& url, void * instancePtr, std::function<void(int)> createdCallback, const bool isTransparent){

	// Handler has an unused browser instance, so use that instead of creating a new one
	if(mHandler && mHandler->hasOrphans()){
		mHandler->useOrphan(createdCallback, url);
		return;
	}

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	// Information used when creating the native window.
	CefWindowInfo window_info;
	HWND hWnd = WindowFromDC(wglGetCurrentDC());
	window_info.SetAsWindowless(hWnd, isTransparent);

	if(mHandler){
		mHandler->addCreatedCallback(instancePtr, createdCallback);
	}

	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, mHandler, url, browser_settings, NULL);
}

}
}