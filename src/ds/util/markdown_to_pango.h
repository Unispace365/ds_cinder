#pragma once
#ifndef DS_UTIL_MARKDOWN_TO_PANGO
#define DS_UTIL_MARKDOWN_TO_PANGO

#include <string>

namespace ds {
namespace ui {

std::wstring markdown_to_pango(const std::wstring& inputMarkdown);
std::string  markdown_to_pango(const std::string& inputMarkdown);
}

} // namespace ds

#endif // DS_UTIL_STRINGUTIL_H_
