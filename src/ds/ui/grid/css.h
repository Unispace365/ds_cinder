#pragma once

#include "cinder/Exception.h"
#include "cinder/Rect.h"

#include <string>

namespace ds::css {

//! Value/Unit pair
class Value {
  public:
	enum Unit { UNDEFINED, PIXELS, PERCENTAGE, FLEX };

	Value() = default;
	Value(float value, Unit unit)
	  : mUnit(unit)
	  , mValue(value) {}

	explicit Value(const std::string& str);
	explicit Value(const char** sInOut);

	float value() const { return mValue; }
	Unit  unit() const { return mUnit; }

	float asUser(float percentOf) const;

	bool isFlex() const { return mUnit == FLEX; }
	bool isFixed() const { return !(mUnit == UNDEFINED || mUnit == FLEX); }

	/// Makes it easier to use, see: GridSprite::sumFlex().
	explicit operator float() const { return mValue; }

	//! Returns whether the value is defined.
	operator bool() const { return !(mUnit == UNDEFINED); }

  private:
	void parse(const char** sInOut);

	Unit  mUnit{UNDEFINED};								   //
	float mValue{std::numeric_limits<float>::quiet_NaN()}; //
};

} // namespace ds::css