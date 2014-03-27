#include "ds/ui/ip/ip_function_list.h"

namespace ds {
namespace ui {
namespace ip {

/**
 * \class ds::ui::ip::FunctionList
 */
FunctionList::FunctionList() {
}

FunctionRef FunctionList::find(const std::string& key) const {
	if (key.empty()) return FunctionRef();
	if (mFunctions.empty()) return FunctionRef();

	auto f = mFunctions.find(key);
	if (f == mFunctions.end()) return FunctionRef();
	return f->second;
}

void FunctionList::add(const std::string& key, const FunctionRef& ref) {
	if (ref.empty()) return;
	mFunctions[key] = ref;
}

} // namespace ip
} // namespace ui
} // namespace ds
