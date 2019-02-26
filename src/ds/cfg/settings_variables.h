#pragma once
#ifndef DS_CFG_SETTINGS_VARIABLES_
#define DS_CFG_SETTINGS_VARIABLES_

#include <string>

namespace ds {
namespace cfg {


class SettingsVariables {

public:


	/// Parses the value if it starts with \#expression, evaluates it, and returns the value as a string
	static std::string parseExpression(const std::string& value);
	static std::string parseAllExpressions(const std::string& value);

	/// Replaces any values starting with $_ with any variables that are found.
	/// Runs until no more variables are found, so be careful with circular references!
	static std::string replaceVariables(const std::string& value);
	static std::string replaceSingleVariable(const std::string& value);

	/// Add a variable to use when replacing variables in layouts
	static void addVariable(const std::string& varName, const std::string& varValue);

};

}  // namespace ui
}  // namespace ds

#endif  // DS_UI_XML_IMPORT_H_
