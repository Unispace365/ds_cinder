#include "private/web_view_listener.h"

#include <iostream>

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

void WebViewListener::OnChangeAddressBar(	Awesomium::WebView* caller,
											const Awesomium::WebURL& url) {
	if (!mAddressChangedFn) return;

	Awesomium::WebString	webstr = url.spec();
	auto					len = webstr.ToUTF8(nullptr, 0);
	if (len < 1) return;
	std::string				str(len+2, 0);
	webstr.ToUTF8(const_cast<char*>(str.c_str()), len);
	mAddressChangedFn(str);
}

} // namespace web
} // namespace ds