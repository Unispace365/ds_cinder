#include "base_impl.h"

namespace ds::cfg::impl {

struct NullComputerInfo : public BaseComputerInfo {

	virtual std::string getAppVersionString() override { return "not found"; };

	virtual std::string getAppProductName() override { return "DS App"; }

	virtual std::string getOsVersion() override { return "Unknown"; }
};

using ComputerInfo = NullComputerInfo;

} // namespace ds::cfg::impl
