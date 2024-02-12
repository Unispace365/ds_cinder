#pragma once

#include <string>

namespace ds::ui {

// Forward declarations.
class Sprite;

//! Value/Unit pair for CSS grid layout. A similar class is also defined in Cinder, but it lacks some features that we
//! are specifically interested in.
class Value {
  public:
	enum Unit {
		UNDEFINED,
		PIXELS,
		PERCENTAGE,
		VIEWPORT_WIDTH,
		VIEWPORT_HEIGHT,
		VIEWPORT_MIN,
		VIEWPORT_MAX,
		FLEX
	}; // Percentage is relative to parent size.
	enum Direction { HORIZONTAL, VERTICAL };

	struct Dimensions {
		float	 percentOf;
		ci::vec2 viewportSize;
	};

	Value() = default;
	Value(float value, Unit unit)
	  : mUnit(unit)
	  , mValue(value) {}

	explicit Value(const std::string& str);
	explicit Value(const char** sInOut);

	[[nodiscard]] float value() const { return mValue; }
	[[nodiscard]] Unit	unit() const { return mUnit; }

	[[nodiscard]] float asUser(const Dimensions& dimensions) const;
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

} // namespace ds::ui