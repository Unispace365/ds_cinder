#include "private/web_view_listener.h"

#include <iostream>
#include "script_translator.h"
#include "Awesomium/WebView.h"
#include <ds/debug/logger.h>
#include <ds/ui/sprite/web.h>

namespace ds {
namespace web {

/**
 * \class ds::web::WebViewListener
 */
WebViewListener::WebViewListener(ds::ui::Web* web)
	: mWeb(web)
	, mAddressChangedFn(nullptr)
{
}

WebViewListener::~WebViewListener() {
}

void WebViewListener::setAddressChangedFn(const std::function<void(const std::string& new_address)>& fn) {
	mAddressChangedFn = fn;
}

void WebViewListener::OnChangeAddressBar(Awesomium::WebView*, const Awesomium::WebURL& url) {
	Awesomium::WebString webstr = url.spec();
//	DS_LOG_INFO("Address changed: " << str_from_webstr(webstr));
	if(mAddressChangedFn) {
		mAddressChangedFn(ds::web::str_from_webstr(webstr));
	}
}

void WebViewListener::OnShowCreatedWebView(	Awesomium::WebView* caller,
											Awesomium::WebView* new_view,
											const Awesomium::WebURL& opener_url,
											const Awesomium::WebURL& target_url,
											const Awesomium::Rect& initial_pos,
											bool is_popup) {
//	std::cout << "OnShowCreatedWebView is_popup=" << is_popup << " target=" << str_from_webstr(target_url.spec()) << std::endl;
}

void WebViewListener::OnAddConsoleMessage(Awesomium::WebView* caller, const Awesomium::WebString& message, int line_number, const Awesomium::WebString& source){
	std::cout << "Console message: " << str_from_webstr(message) << " Line: " << line_number << " From: " << str_from_webstr(source) <<  std::endl;
}

/**
 * \class ds::web::WebLoadListener
 */
WebLoadListener::WebLoadListener(ds::ui::Web* web)
	: mWeb(web)
	, mOnDocumentReadyFn(nullptr)
{
}

WebLoadListener::~WebLoadListener() {
}

void WebLoadListener::setOnDocumentReady(const std::function<void(const std::string& url)>& fn) {
	mOnDocumentReadyFn = fn;
}


void WebLoadListener::OnBeginLoadingFrame(	Awesomium::WebView* caller, int64 frame_id,
											bool is_main_frame, const Awesomium::WebURL& url,
											bool is_error_page) {
//	DS_LOG_INFO("OnBeginLoadingFrame is_main=" << is_main_frame << " url=" << str_from_webstr(url.spec()));
}

void WebLoadListener::OnFailLoadingFrame(	Awesomium::WebView* caller, int64 frame_id,
											bool is_main_frame, const Awesomium::WebURL& url,
											int error_code, const Awesomium::WebString& error_desc) {
	DS_LOG_WARNING("OnFailLoadingFrame is_main=" << is_main_frame << " url=" << str_from_webstr(url.spec()));
	if(mWeb)
	{
		std::string msg("Failed to load URL ");
		msg.append(str_from_webstr(url.spec()));
		mWeb->setErrorMessage(msg);
	}
}

void WebLoadListener::OnFinishLoadingFrame(	Awesomium::WebView* caller, int64 frame_id,
													bool is_main_frame, const Awesomium::WebURL& url) {
//	DS_LOG_INFO("OnFinishLoadingFrame is_main=" << is_main_frame << " url=" << str_from_webstr(url.spec()));
}

void WebLoadListener::OnDocumentReady(Awesomium::WebView* v, const Awesomium::WebURL& url) {
	Awesomium::WebString webstr = url.spec();
//	DS_LOG_INFO("On document ready: " << str_from_webstr(webstr));
	if(mOnDocumentReadyFn){
		mOnDocumentReadyFn(ds::web::str_from_webstr(webstr));
	}
}


void WebProcessListener::OnUnresponsive(Awesomium::WebView* caller){
	DS_LOG_WARNING("Web view unresponsive, url: " << str_from_webstr(caller->url().spec()));
}

void WebProcessListener::OnResponsive(Awesomium::WebView* caller){
//	DS_LOG_INFO("Web view responsive, url: " << str_from_webstr(caller->url().spec()));
}

void WebProcessListener::OnCrashed(Awesomium::WebView* caller, Awesomium::TerminationStatus status){
	DS_LOG_WARNING("Web view crashed, url: " << str_from_webstr(caller->url().spec()));
}

void WebProcessListener::OnLaunch(Awesomium::WebView* caller){
//	DS_LOG_INFO("On process launch " << str_from_webstr(caller->url().spec()));
}

WebProcessListener::WebProcessListener(ds::ui::Web* web)
	: mWeb(web)
{
}

WebProcessListener::~WebProcessListener(){

}


void WebDialogListener::OnShowFileChooser(Awesomium::WebView* caller, const Awesomium::WebFileChooserInfo& chooser_info){
	DS_LOG_INFO("Show file chooser " << str_from_webstr(caller->url().spec()));
}

void WebDialogListener::OnShowLoginDialog(Awesomium::WebView* caller, const Awesomium::WebLoginDialogInfo& dialog_info){
	DS_LOG_INFO("Show Login Dialog " << str_from_webstr(caller->url().spec()));
}

void WebDialogListener::OnShowCertificateErrorDialog(Awesomium::WebView* caller, bool is_overridable, const Awesomium::WebURL& url, Awesomium::CertError error){
	if(caller && is_overridable){
		DS_LOG_WARNING("Certificate error, overriding and display page anyways: " << str_from_webstr(caller->url().spec()));
		caller->DidOverrideCertificateError();
	} else {
		DS_LOG_WARNING("Certificate error that could not be recovered from: " << str_from_webstr(caller->url().spec()));
	}
}

void WebDialogListener::OnShowPageInfoDialog(Awesomium::WebView* caller, const Awesomium::WebPageInfo& page_info){
	DS_LOG_INFO("Show page info dialog " << str_from_webstr(caller->url().spec()));
}

WebDialogListener::WebDialogListener(ds::ui::Web* web)
	: mWeb(web)
{
}

WebDialogListener::~WebDialogListener(){

}


void WebMenuListener::OnShowPopupMenu(Awesomium::WebView* caller, const Awesomium::WebPopupMenuInfo& menu_info){
	//TODO: set a callback?
 	//std::cout << "show popup! " << std::endl;
//  	for(int i = 0; i < menu_info.items.size(); i++){
// 		std::cout << "Menu item: " << str_from_webstr(menu_info.items[i].label) << std::endl;
//  	}
}

void WebMenuListener::OnShowContextMenu(Awesomium::WebView* caller, const Awesomium::WebContextMenuInfo& menu_info){
//	std::cout << "Context menu! " << std::endl;

}

} // namespace web
} // namespace ds