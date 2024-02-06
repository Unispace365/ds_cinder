#pragma once

#include <string>

namespace ds::ui {
class Sprite;
}

namespace ds::css {

//! Value/Unit pair
class Value {
  public:
	enum Unit { UNDEFINED, PIXELS, PERCENTAGE, FLEX }; // Percentage is relative to parent size.
	enum Direction { HORIZONTAL, VERTICAL };

	Value() = default;
	Value(float value, Unit unit)
	  : mUnit(unit)
	  , mValue(value) {}

	explicit Value(const std::string& str);
	explicit Value(const char** sInOut);

	[[nodiscard]] float value() const { return mValue; }
	[[nodiscard]] Unit  unit() const { return mUnit; }

	[[nodiscard]] float asUser(float percentOf) const;
	[[nodiscard]] float asUser(const ds::ui::Sprite* sprite, Direction direction) const;

	[[nodiscard]] bool isDefined() const { return mUnit != UNDEFINED; }
	[[nodiscard]] bool isFlex() const { return mUnit == FLEX; }
	[[nodiscard]] bool isFixed() const { return !(mUnit == UNDEFINED || mUnit == FLEX); }

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