#include "private/js_method_handler.h"

#include <iostream>
#include "Awesomium/WebView.h"
#include "script_translator.h"

namespace ds {
namespace web {

/**
 * \class ds::web::JsMethodHandler
 */
JsMethodHandler::JsMethodHandler(ds::ui::Web* web)
	: mWeb(web)
	, mDomIsReady(false)
{
}

JsMethodHandler::~JsMethodHandler() {
}

void JsMethodHandler::setDomIsReady(Awesomium::WebView& v) {
	mDomIsReady = true;
	for (auto it=mRegisteredMethods.begin(), end=mRegisteredMethods.end(); it!=end; ++it) {
		doRegisterMethod(v, it->mClassName, it->mMethodName);
	}
}

void JsMethodHandler::registerMethod(	Awesomium::WebView& v, const std::string& class_name, const std::string& method_name,
										const std::function<void(const ds::web::ScriptTree&)>& fn) {
	mRegisteredMethods.push_back(Method(class_name, method_name, fn));
	if (mDomIsReady) {
		doRegisterMethod(v, class_name, method_name);
	}
}

void JsMethodHandler::OnMethodCall(	Awesomium::WebView*, unsigned int remote_object_id,
									const Awesomium::WebString& method_name, const Awesomium::JSArray& args) {
	const std::string					method = ds::web::str_from_webstr(method_name);
	for (auto it=mRegisteredMethods.begin(), end=mRegisteredMethods.end(); it!=end; ++it) {
		if (remote_object_id == it->mRemoteId && method == it->mMethodName) {
			if (it->mFn) {
				ds::web::ScriptTree		tree(ds::web::tree_from_jsarray(args));
				it->mFn(tree);
			}
			return;
		}
	}
}

Awesomium::JSValue JsMethodHandler::OnMethodCallWithReturnValue(
									Awesomium::WebView* caller, unsigned int remote_object_id,
									const Awesomium::WebString& method_name, const Awesomium::JSArray& args) {
	return Awesomium::JSValue();
}

void JsMethodHandler::doRegisterMethod(Awesomium::WebView& v, const std::string& class_name, const std::string& method_name) {
	Awesomium::JSValue jsv  = v.CreateGlobalJavascriptObject(ds::web::webstr_from_str(class_name));
	if (jsv.IsObject()) {
		Awesomium::JSObject&		obj(jsv.ToObject());
		obj.SetCustomMethod(ds::web::webstr_from_str(method_name), false);
		for (auto it=mRegisteredMethods.begin(), end=mRegisteredMethods.end(); it!=end; ++it) {
			if (it->mClassName == class_name && it->mMethodName == method_name) {
				it->mRemoteId = obj.remote_id();
			}
		}
	}
}

} // namespace web
} // namespace ds