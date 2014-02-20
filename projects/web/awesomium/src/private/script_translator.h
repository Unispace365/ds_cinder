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

ScriptTree			tree_from_jsvalue(const Awesomium::JSValue&);
Awesomium::JSArray	jsarray_from_tree(const ScriptTree&);

} // namespace web
} // namespace ds

#endif // PRIVATE_SCRIPTTRANSLATOR_H_