// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_app.h"

#include <string>

#include "simple_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

#include <ds/debug/debug_defines.h>


SimpleApp::SimpleApp() {
}

#include <iostream>
void SimpleApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line){
	std::cout << "before command line" << std::endl;
	std::string argy = "--disable-extensions";
	// This would prevent a crash in debug:
	command_line->AppendSwitch(CefString(argy));
	command_line->AppendSwitchWithValue(CefString("enable-system-flash"), CefString("1"));
}

void SimpleApp::OnContextInitialized() {
	CEF_REQUIRE_UI_THREAD();
	std::cout << "on context initialized" << std::endl;

	CefRefPtr<CefCommandLine> command_line =
		CefCommandLine::GetGlobalCommandLine();

	const bool disable_extensions = command_line->HasSwitch("disable-extensions");

	// SimpleHandler implements browser-level callbacks.
	mHandler = CefRefPtr<SimpleHandler>(new SimpleHandler());

	
}

void SimpleApp::createBrowser(const std::string& url){
	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	// Information used when creating the native window.
	CefWindowInfo window_info;

	// On Windows we need to specify certain flags that will be passed to
	// CreateWindowEx().
	window_info.SetAsPopup(NULL, "cefsimple");
	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, mHandler, url, browser_settings,
								  NULL);
}
