/**
 * @file projects/essentials/src/ds/ui/util/text_model.h
 *
 * @brief ...
 * @author Luke Purcell
 * @date 08/11/2018
 */

#pragma once
#ifndef DS_CINDER_PROJECTS_ESSENTIALS_SRC_DS_UI_UTIL_TEXT_MODEL
#define DS_CINDER_PROJECTS_ESSENTIALS_SRC_DS_UI_UTIL_TEXT_MODEL

#include <string>
#include <vector>

namespace ds::model {
class ContentModelRef;
}

namespace ds::ui {


/// Process a string:
///   Values inside '{'..'}' will be replaced with the corresponding property from the content model
///   Then anything within 'fn('..')' will try to find a corresponding 'function' and replace
std::string processTextModel(const std::string& format, ds::model::ContentModelRef& model);

std::string processTextFunction(std::vector<std::string>& fnSplit);

std::string utcFormat(const std::string& value, const std::string& outFmt, const std::string& parseFmt = "",
					  const bool isLocal = false);

} // namespace ds::ui

#endif
