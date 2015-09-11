#pragma once
#ifndef PRIVATE_JSMETHODHANDLER_H_
#define PRIVATE_JSMETHODHANDLER_H_

#include <functional>
#include "Awesomium/JSObject.h"
#include "Awesomium/JSValue.h"
#include "ds/script/web_script.h"

namespace Awesomium {
class WebView;
}

namespace ds {
namespace ui {
class Web;
}
namespace web {

/**
 * \class ds::web::JsMethodHandler
 * \brief Handle View callbacks.
 */
class JsMethodHandler : public Awesomium::JSMethodHandler {
public:
	JsMethodHandler(ds::ui::Web* web);
	virtual ~JsMethodHandler();

	// Provide a convenience for registering methods: They can't be registered
	// until the DOM is created, so buffer up and wait for that to be the case.
	void						setDomIsReady(	Awesomium::WebView&);
	void						registerMethod(	Awesomium::WebView&, const std::string& class_name, const std::string& method_name,
												const std::function<void(const ds::web::ScriptTree&)>&);

	virtual void				OnMethodCall(	Awesomium::WebView* caller, unsigned int remote_object_id,
												const Awesomium::WebString& method_name, const Awesomium::JSArray& args);
	virtual Awesomium::JSValue	OnMethodCallWithReturnValue(
												Awesomium::WebView* caller, unsigned int remote_object_id,
												const Awesomium::WebString& method_name, const Awesomium::JSArray& args);

private:
	ds::ui::Web*				mWeb;
	void						doRegisterMethod(Awesomium::WebView&, const std::string& class_name, const std::string& method_name);

	// Buffer up any method registrations until the DOM is ready.
	bool						mDomIsReady;
	class Method {
	public:
		Method(	const std::string& class_name, const std::string& method_name,
				const std::function<void(const ds::web::ScriptTree&)>& fn) : mClassName(class_name), mMethodName(method_name), mRemoteId(0), mFn(fn) { }
		std::string				mClassName,
								mMethodName;
		unsigned int			mRemoteId;
		std::function<void(const ds::web::ScriptTree&)>
								mFn;
	};
	std::vector<Method>			mRegisteredMethods;
};

} // namespace web
} // namespace ds

#endif // PRIVATE_JSMETHODHANDLER_H_