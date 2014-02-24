#include "web_script.h"

#include <ds/util/string_util.h>

namespace ds {
namespace web {

/**
 * \class ds::web::ScriptValue
 */
ScriptValue::ScriptValue()
		: mType(kString) {
}

ScriptValue::ScriptValue(const double v)
		: mType(kFloat)
		, mValue(ds::value_to_wstring(v)) {
}

ScriptValue::ScriptValue(const int32_t v)
		: mType(kInteger)
		, mValue(ds::value_to_wstring(v)) {
}

ScriptValue::ScriptValue(const std::wstring& v)
		: mType(kString)
		, mValue(v) {
}

double ScriptValue::asFloat() const {
	double		v = 0.0;
	ds::wstring_to_value(mValue, v);
	return v;
}

int ScriptValue::asInt() const {
	int			v = 0;
	ds::wstring_to_value(mValue, v);
	return v;
}

const std::wstring& ScriptValue::asString() const {
	return mValue;
}

/**
 * \class ds::web::ScriptTree
 */
ScriptTree::ScriptTree() {
}

bool ScriptTree::empty() {
	return mValue.empty();
}

void ScriptTree::clear() {
	mValue.clear();
}

size_t ScriptTree::size() const {
	return mValue.size();
}

void ScriptTree::push_back(const ScriptValue& v) {
	mValue.push_back(v);
}

const ScriptValue& ScriptTree::at(const size_t index) const {
	if (index >= mValue.size()) throw std::runtime_error("ScriptTree::at() invalid index");
	return mValue[index];
}

int ScriptTree::intAt(const size_t index, const int on_missing) const {
	if (index >= mValue.size()) return on_missing;
	if (mValue[index].mType != ScriptValue::kInteger) return on_missing;
	int ans = 0;
	if (!ds::wstring_to_value(mValue[index].mValue, ans)) return on_missing;
	return ans;
}

} // namespace web
} // namespace ds
