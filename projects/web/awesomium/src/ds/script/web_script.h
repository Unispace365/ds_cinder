#pragma once
#ifndef DS_SCRIPT_WEBSCRIPT_H_
#define DS_SCRIPT_WEBSCRIPT_H_

#include <stdint.h>
#include <string>
#include <vector>

namespace Awesomium {
class WebCore;
class WebView;
}

namespace ds {
namespace web {

/**
 * \class ds::web::ScriptValue
 * \brief A script value is just a string with an associated type.
 * All values are represented as strings, the type is optional for
 * compatibility with other APIs.
 */
class ScriptValue {
public:
	enum				Type { kFloat, kInteger, kString };
	ScriptValue();
	explicit ScriptValue(const double);
	explicit ScriptValue(const int32_t);
	explicit ScriptValue(const std::wstring&);

	double				asFloat() const;
	int					asInt() const;
	const std::wstring&	asString() const;

	Type				mType;
	std::wstring		mValue;
};

/**
 * \class ds::web::ScriptTree
 * \brief A collection of script values. Currently not hierarchical.
 */
class ScriptTree {
public:
	ScriptTree();

	bool						empty();
	void						clear();
	size_t						size() const;
	void						push_back(const ScriptValue&);
	const ScriptValue&			at(const size_t index) const;
	// Convenient type accessing
	int							intAt(const size_t index, const int on_missing = -1) const;

private:
	std::vector<ScriptValue>	mValue;
};

} // namespace web
} // namespace ds

#endif // DS_SCRIPT_WEBSCRIPT_H_
