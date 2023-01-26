#pragma once
#include "stdafx.h"

namespace ds::cfg::impl {

struct BaseComputerInfo {

	/// Returns the application verion in xx.xx.xx.xx format (if a version is set)
	virtual std::string getAppVersionString() = 0;

	/// Returns the application product name (if set) otherwise returns generic "DS App"
	virtual std::string getAppProductName() = 0;

	/// Returns the OS Name + Version (or Unknown)
	virtual std::string getOsVersion() = 0;

	virtual std::string getOpenGlVendor() = 0;

	virtual std::string getOpenglVersion() = 0;
};

} // namespace ds::cfg::impl

