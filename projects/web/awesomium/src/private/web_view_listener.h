#pragma once
#ifndef PRIVATE_WEBVIEWLISTENER_H_
#define PRIVATE_WEBVIEWLISTENER_H_

#include <functional>
#include "Awesomium/WebURL.h"
#include "Awesomium/WebViewListener.h"

namespace ds {
namespace web {

/**
 * \class ds::web::WebViewListener
 * \brief The engine service object that provides access to the
 * web service.
 */
class WebViewListener : public Awesomium::WebViewListener::View {
public:
	WebViewListener();
	virtual ~WebViewListener();

	void					setAddressChangedFn(const std::function<void(const std::string& new_address)>&);

	virtual void OnChangeTitle(Awesomium::WebView* caller,
                             const Awesomium::WebString& title) { }
	virtual void OnChangeAddressBar(Awesomium::WebView* caller,
                                  const Awesomium::WebURL& url);
	virtual void OnChangeTooltip(Awesomium::WebView* caller,
                               const Awesomium::WebString& tooltip) { }
	virtual void OnChangeTargetURL(Awesomium::WebView* caller,
                                 const Awesomium::WebURL& url) { }
	virtual void OnChangeCursor(Awesomium::WebView* caller,
                              Awesomium::Cursor cursor) { }
	virtual void OnChangeFocus(Awesomium::WebView* caller,
                             Awesomium::FocusedElementType focused_type) { }
	virtual void OnAddConsoleMessage(Awesomium::WebView* caller,
                                   const Awesomium::WebString& message,
                                   int line_number,
                                   const Awesomium::WebString& source) { }
	virtual void OnShowCreatedWebView(Awesomium::WebView* caller,
                                    Awesomium::WebView* new_view,
                                    const Awesomium::WebURL& opener_url,
                                    const Awesomium::WebURL& target_url,
                                    const Awesomium::Rect& initial_pos,
                                    bool is_popup) { }

private:
	std::function<void(const std::string& new_address)>
									mAddressChangedFn;
};

} // namespace web
} // namespace ds

#endif // PRIVATE_WEBVIEWLISTENER_H_