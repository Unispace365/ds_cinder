#pragma once
#ifndef DS_UTIL_MARKDOWN_TO_PANGO
#define DS_UTIL_MARKDOWN_TO_PANGO

#include <string>

namespace ds { namespace ui {

	/// TODO: it'd be cool if this supported custom callbacks for certain elements for further styling
	///			For instance, you could specify your own font or background color for code blocks or headers, etc
	std::wstring markdown_to_pango(const std::wstring& inputMarkdown);
	std::string	 markdown_to_pango(const std::string& inputMarkdown);
}} // namespace ds::ui

#endif // DS_UTIL_STRINGUTIL_H_
