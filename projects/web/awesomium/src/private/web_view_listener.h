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
 * \brief Handle View callbacks.
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
                                    bool is_popup);

private:
	std::function<void(const std::string& new_address)>
									mAddressChangedFn;
};

/**
 * \class ds::web::WebLoadListener
 * \brief Handle Load callbacks.
 */
class WebLoadListener : public Awesomium::WebViewListener::Load {
public:
	WebLoadListener();
	virtual ~WebLoadListener();

	void					setOnDocumentReady(const std::function<void(const std::string& url)>&);

	virtual void			OnBeginLoadingFrame(	Awesomium::WebView* caller, int64 frame_id,
													bool is_main_frame, const Awesomium::WebURL& url,
													bool is_error_page);
	virtual void			OnFailLoadingFrame(		Awesomium::WebView* caller, int64 frame_id,
													bool is_main_frame, const Awesomium::WebURL& url,
													int error_code, const Awesomium::WebString& error_desc);
	virtual void			OnFinishLoadingFrame(	Awesomium::WebView* caller, int64 frame_id,
													bool is_main_frame, const Awesomium::WebURL& url);
	virtual void			OnDocumentReady(		Awesomium::WebView* caller, const Awesomium::WebURL& url);

private:
	std::function<void(const std::string& url)>
							mOnDocumentReadyFn;
};

} // namespace web
} // namespace ds

#endif // PRIVATE_WEBVIEWLISTENER_H_