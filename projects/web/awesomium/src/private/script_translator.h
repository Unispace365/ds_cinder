#pragma once
#ifndef PRIVATE_SCRIPTTRANSLATOR_H_
#define PRIVATE_SCRIPTTRANSLATOR_H_

#include <Awesomium/JSValue.h>
#include <ds/script/web_script.h>

namespace ds {
namespace web {

/*
 * Utilities for translating between awesomimum JSValues and script values.
 */
ScriptTree				tree_from_jsarray(const Awesomium::JSArray&);
ScriptTree				tree_from_jsvalue(const Awesomium::JSValue&);
Awesomium::JSArray		jsarray_from_tree(const ScriptTree&);

/* Convenience
 */
std::string				str_from_webstr(const Awesomium::WebString&);
std::wstring			wstr_from_webstr(const Awesomium::WebString&);
Awesomium::WebString	webstr_from_str(const std::string&);
Awesomium::WebString	webstr_from_wstr(const std::wstring&);

} // namespace web
} // namespace ds

#endif // PRIVATE_SCRIPTTRANSLATOR_H_