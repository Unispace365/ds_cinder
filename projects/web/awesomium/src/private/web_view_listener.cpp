#include "private/web_view_listener.h"

#include <iostream>
#include "script_translator.h"
#include "Awesomium/WebView.h"

namespace ds {
namespace web {

/**
 * \class ds::web::WebViewListener
 */
WebViewListener::WebViewListener()
		: mAddressChangedFn(nullptr) {
}

WebViewListener::~WebViewListener() {
}

void WebViewListener::setAddressChangedFn(const std::function<void(const std::string& new_address)>& fn) {
	mAddressChangedFn = fn;
}

void WebViewListener::OnChangeAddressBar(Awesomium::WebView*, const Awesomium::WebURL& url) {
	if (!mAddressChangedFn) return;

	Awesomium::WebString	webstr = url.spec();
	mAddressChangedFn(ds::web::str_from_webstr(webstr));
}

void WebViewListener::OnShowCreatedWebView(	Awesomium::WebView* caller,
											Awesomium::WebView* new_view,
											const Awesomium::WebURL& opener_url,
											const Awesomium::WebURL& target_url,
											const Awesomium::Rect& initial_pos,
											bool is_popup) {
//	std::cout << "OnShowCreatedWebView is_popup=" << is_popup << " target=" << str_from_webstr(target_url.spec()) << std::endl;
}

/**
 * \class ds::web::WebLoadListener
 */
WebLoadListener::WebLoadListener()
		: mOnDocumentReadyFn(nullptr) {
}

WebLoadListener::~WebLoadListener() {
}

void WebLoadListener::setOnDocumentReady(const std::function<void(const std::string& url)>& fn) {
	mOnDocumentReadyFn = fn;
}


void WebLoadListener::OnBeginLoadingFrame(	Awesomium::WebView* caller, int64 frame_id,
											bool is_main_frame, const Awesomium::WebURL& url,
											bool is_error_page) {
//	std::cout << "OnBeginLoadingFrame is_main=" << is_main_frame << " url=" << str_from_webstr(url.spec()) << std::endl;
}

void WebLoadListener::OnFailLoadingFrame(	Awesomium::WebView* caller, int64 frame_id,
											bool is_main_frame, const Awesomium::WebURL& url,
											int error_code, const Awesomium::WebString& error_desc) {
//	std::cout << "OnFailLoadingFrame is_main=" << is_main_frame << " url=" << str_from_webstr(url.spec()) << std::endl;
}

void WebLoadListener::OnFinishLoadingFrame(	Awesomium::WebView* caller, int64 frame_id,
													bool is_main_frame, const Awesomium::WebURL& url) {
//	std::cout << "OnFinishLoadingFrame is_main=" << is_main_frame << " url=" << str_from_webstr(url.spec()) << std::endl;
}

void WebLoadListener::OnDocumentReady(Awesomium::WebView* v, const Awesomium::WebURL& url) {
	if (!mOnDocumentReadyFn) return;

	Awesomium::WebString	webstr = url.spec();
	mOnDocumentReadyFn(ds::web::str_from_webstr(webstr));
}

} // namespace web
} // namespace ds