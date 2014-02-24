#include "private/script_translator.h"

#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

namespace ds {
namespace web {

/*
 * tree-from-jsarray
 */
ScriptTree	tree_from_jsarray(const Awesomium::JSArray& jsa) {
	DS_LOG_WARNING("ds::web::tree_from_jsarray() unimplemented");
	ScriptTree		tree;
	return tree;
}

/*
 * tree-from-value
 */
ScriptTree	tree_from_jsvalue(const Awesomium::JSValue& jsv) {
	ScriptTree		tree;
	if (jsv.IsString()) {
		tree.push_back(ScriptValue(wstr_from_webstr(jsv.ToString())));
	} else if (jsv.IsInteger()) {
		tree.push_back(ScriptValue(jsv.ToInteger()));
	}
	return tree;
}

/*
 * jsarray-from-tree
 */
Awesomium::JSArray	jsarray_from_tree(const ScriptTree& tree) {
	Awesomium::JSArray		jsa;
	for (size_t k=0; k<tree.size(); ++k) {
		const ScriptValue&	v(tree.at(k));
		if (v.mType == v.kInteger) {
			jsa.Push(Awesomium::JSValue(v.asInt()));
		} else if (v.mType == v.kFloat) {
			jsa.Push(Awesomium::JSValue(v.asFloat()));
		} else if (v.mType == v.kString) {
			jsa.Push(Awesomium::JSValue(webstr_from_wstr(v.asString())));
		}
	}
	return jsa;
}

/*
 * String conversion
 */
std::string			str_from_webstr(const Awesomium::WebString& webstr) {
	if (webstr.length() > 0) {
		auto			len = webstr.ToUTF8(nullptr, 0);
		if (len < 1) return std::string();
		std::string		str(len, 0);
		webstr.ToUTF8(const_cast<char*>(str.c_str()), len);
		return str;
	}
	return std::string();
}

std::wstring			wstr_from_webstr(const Awesomium::WebString& webstr) {
	return ds::wstr_from_utf8(str_from_webstr(webstr));
}

Awesomium::WebString	webstr_from_str(const std::string& str) {
	if (str.empty()) return Awesomium::WebString();
	return Awesomium::WebString::CreateFromUTF8(str.c_str(), str.size());
}

Awesomium::WebString	webstr_from_wstr(const std::wstring& wstr) {
	return webstr_from_str(ds::utf8_from_wstr(wstr));
}

} // namespace web
} // namespace ds