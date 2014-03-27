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

void WebLoadListener::OnDocumentReady(Awesomium::WebView* v, const Awesomium::WebURL& url) {
	if (!mOnDocumentReadyFn) return;

	Awesomium::WebString	webstr = url.spec();
	mOnDocumentReadyFn(ds::web::str_from_webstr(webstr));
}

} // namespace web
} // namespace ds