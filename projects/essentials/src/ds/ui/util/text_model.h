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


namespace ds {
namespace model {
class ContentModelRef;
}
namespace ui {


std::string processTextModel(const std::string& format, ds::model::ContentModelRef& model);

std::string processTextFunction(std::vector<std::string>& fnSplit);

std::string utcFormat(const std::string& value, const std::string& outFmt, const std::string& parseFmt = "");

}  // namespace ui
}  // namespace ui

#endif
