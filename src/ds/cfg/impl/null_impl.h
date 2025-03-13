#pragma once

#include "base_impl.h"

namespace ds::cfg::impl {

struct NullComputerInfo : public BaseComputerInfo {

	std::string getAppVersionString() override { return "not found"; };

	std::string getAppProductName() override { return "DS App"; }

	std::string getOsVersion() override { return "Unknown"; }

	std::string getOpenGlVendor() override { return "Unknown"; }

	std::string getOpenGlVersion() override { return "Unknown"; }
};

using ComputerInfo = NullComputerInfo;

} // namespace ds::cfg::impl
