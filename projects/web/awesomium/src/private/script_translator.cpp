#include "private/script_translator.h"

#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

namespace ds {
namespace web {

namespace {
std::wstring			str_from_webstr(const Awesomium::WebString& webstr) {
	if (webstr.length() > 1020) {
		DS_LOG_ERROR("ds::web::script_translator.cpp::str_from_webstr() currently limited in string size");
		return std::wstring();
	}
	if (webstr.length() > 0) {
		char			buf[1024];
		const int	size = webstr.ToUTF8(buf, 1020);
		std::string		str(buf);
		return ds::wstr_from_utf8(str);
	}
	return std::wstring();
}

Awesomium::WebString	webstr_from_str(const std::wstring& wstr) {
	std::string			str = ds::utf8_from_wstr(wstr);
	if (str.empty()) return Awesomium::WebString();
	return Awesomium::WebString::CreateFromUTF8(str.c_str(), str.size());
}

}

/*
 * tree-from-value
 */
ScriptTree	tree_from_jsvalue(const Awesomium::JSValue& jsv) {
	ScriptTree		tree;
	if (jsv.IsString()) {
		tree.push_back(ScriptValue(str_from_webstr(jsv.ToString())));
	} else if (jsv.IsInteger()) {
		tree.push_back(ScriptValue(jsv.ToInteger()));
	}
	return tree;
}

Awesomium::JSArray	jsarray_from_tree(const ScriptTree& tree) {
	Awesomium::JSArray		jsa;
	for (size_t k=0; k<tree.size(); ++k) {
		const ScriptValue&	v(tree.at(k));
		if (v.mType == v.kInteger) {
			jsa.Push(Awesomium::JSValue(v.asInt()));
		} else if (v.mType == v.kFloat) {
			jsa.Push(Awesomium::JSValue(v.asFloat()));
		} else if (v.mType == v.kString) {
			jsa.Push(Awesomium::JSValue(webstr_from_str(v.asString())));
		}
	}
	return jsa;
}

} // namespace web
} // namespace ds