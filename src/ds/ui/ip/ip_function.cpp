#include "ds/ui/ip/ip_function.h"

namespace ds {
namespace ui {
namespace ip {

/**
 * \class ds::ui::ip::Function
 */
Function::Function() {
}

Function::~Function() {
}

/**
 * \class ds::ui::ip::Function
 */
FunctionRef::FunctionRef() {
}

FunctionRef::FunctionRef(const std::shared_ptr<Function>& fn)
		: mFn(fn) {
}

bool FunctionRef::empty() const {
	return !mFn;
}

void FunctionRef::clear() {
	mFn.reset();
}

void FunctionRef::on(const std::string& parameters, ci::Surface8u& s) const {
	if (mFn) mFn->on(parameters, s);
}

} // namespace ip
} // namespace ui
} // namespace ds
