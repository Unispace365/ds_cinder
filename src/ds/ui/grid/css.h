#pragma once

#include "cinder/Exception.h"
#include "cinder/Rect.h"

#include <string>

namespace ds::css {

template <typename T>
class Exception : public ci::Exception {
  public:
	Exception() = default;
	Exception(T data, const std::string& description)
	  : ci::Exception(description)
	  , mData(std::move(data)) {}

	T&		 data() { return mData; }
	const T& data() const noexcept { return mData; }

  private:
	T mData;
};

using ParseException = Exception<std::string>;

constexpr double EPSILON_VALUE = 4.37114e-05;

inline bool approxZero(float n, float epsilon = float(EPSILON_VALUE)) {
	return std::abs(n) < epsilon;
}

inline bool approxZero(double n, double epsilon = EPSILON_VALUE) {
	return std::abs(n) < epsilon;
}

inline float roundToZero(float n, float epsilon = float(EPSILON_VALUE)) {
	return approxZero(n, epsilon) ? 0.0f : n;
}

inline double roundToZero(double n, double epsilon = EPSILON_VALUE) {
	return approxZero(n, epsilon) ? 0.0 : n;
}

inline bool approxEqual(float a, float b, float epsilon = float(EPSILON_VALUE)) {
	return std::abs(b - a) < epsilon;
}

inline bool approxEqual(double a, double b, double epsilon = EPSILON_VALUE) {
	return std::abs(b - a) < epsilon;
}

inline bool isNumeric(char c) {
	return (c >= '0' && c <= '9') || c == '.' || c == '-' || c == 'e' || c == 'E' || c == '+';
}

inline void skipSpace(const char** sInOut) {
	while (**sInOut && isspace(**sInOut))
		++(*sInOut);
}

inline void skipSpaceOrComma(const char** sInOut) {
	while (**sInOut && (isspace(**sInOut) || **sInOut == ','))
		++(*sInOut);
}

inline void skipSpaceOrParenthesis(const char** sInOut) {
	while (**sInOut && (isspace(**sInOut) || **sInOut == '(' || **sInOut == ')'))
		++(*sInOut);
}

inline void skipUntil(const char** sInOut, char c) {
	while (**sInOut && **sInOut != c)
		++(*sInOut);
}

inline std::string fetchUntil(const char** sInOut, char c) {
	const char* from = *sInOut;
	skipUntil(sInOut, c);
	return std::string(from, *sInOut - from);
}

inline std::string fetchWord(const char** sInOut) {
	const char* from = *sInOut;
	while (**sInOut && !isspace(**sInOut))
		++(*sInOut);
	return std::string(from, *sInOut - from);
}

inline float parseFloat(const char** sInOut) {
	char   temp[256];
	size_t i = 0;
	skipSpaceOrComma(sInOut);
	const char* s = *sInOut;
	if (!s) throw ParseException(*sInOut, "Invalid parameter");
	if (isNumeric(*s)) {
		while (*s == '-' || *s == '+') {
			if (i < sizeof(temp)) temp[i++] = *s;
			s++;
		}
		bool parsingExponent  = false;
		bool startingExponent = false;
		bool seenDecimal	  = false;
		while (*s && (parsingExponent || (*s != '-' && *s != '+')) && isNumeric(*s)) {
			startingExponent = false;
			if (*s == '.' && seenDecimal)
				break;
			else if (*s == '.')
				seenDecimal = true;
			if (i < sizeof(temp)) temp[i++] = *s;
			if (*s == 'e' || *s == 'E') {
				parsingExponent	 = true;
				startingExponent = true;
			} else
				parsingExponent = false;
			s++;
		}
		if (startingExponent) { // if we got a false positive on an exponent, for example due to "ex" or "em" unit, back
								// up one
			--i;
			--s;
		}
		*sInOut = s;
		temp[i] = 0;
		return float(atof(temp));
	}

	throw ParseException(*sInOut, "Error parsing float");
}

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

using ValueException = Exception<Value>;

class SizingFn {
  public:
	enum Unit { UNDEFINED, FIXED, MIN_CONTENT, MAX_CONTENT };

	SizingFn() = default;

	explicit SizingFn(const std::string& str);
	explicit SizingFn(const char** sInOut);

	const Value& value() const { return mValue; }
	Unit		 unit() const { return mUnit; }

	bool isFixed() const { return mUnit == FIXED && mValue.isFixed(); }
	bool isIntrinsic() const { return !isFixed(); }
	bool isMinContent() const { return mUnit == MIN_CONTENT; }
	bool isMaxContent() const { return mUnit == MAX_CONTENT; }
	bool isFlex() const { return mUnit == FIXED && mValue.isFlex(); }

	//! Returns whether the sizing function is defined.
	operator bool() const { return mUnit != UNDEFINED; }

  private:
	void parse(const char** sInOut);

	Unit  mUnit{UNDEFINED};
	Value mValue;
};

class GridLine {
  public:
	enum Unit { AUTO, SPAN, SINGLE };

	int				   value() const { return mValue; }
	Unit			   unit() const { return mUnit; }
	const std::string& ident() const { return mIdent; }

	bool hasIdent() const { return !mIdent.empty(); }
	bool hasValue() const { return mUnit == SINGLE; }
	bool isSpan() const { return mUnit == SPAN; }
	bool isAuto() const { return mUnit == AUTO; }

  private:
	void parse(const char** sInOut);

	std::string mIdent;
	int			mValue{0};
	Unit		mUnit{AUTO};
};

class PreserveAspectRatio {
  public:
	enum Align {
		ALIGN_NONE,
		ALIGN_X_MIN_Y_MIN,
		ALIGN_X_MID_Y_MIN,
		ALIGN_X_MAX_Y_MIN,
		ALIGN_X_MIN_Y_MID,
		ALIGN_X_MID_Y_MID,
		ALIGN_X_MAX_Y_MID,
		ALIGN_X_MIN_Y_MAX,
		ALIGN_X_MID_Y_MAX,
		ALIGN_X_MAX_Y_MAX
	};
	enum MeetOrSlice { MEET, SLICE };

	PreserveAspectRatio() = default;

	explicit PreserveAspectRatio(Align align, MeetOrSlice meetOrSlice = MEET)
	  : align(align)
	  , meetOrSlice(meetOrSlice) {}

	explicit PreserveAspectRatio(const std::string& str);
	explicit PreserveAspectRatio(const char** sInOut);

	glm::mat3x2 calcTransform(const ci::Rectf& element, const ci::Rectf& viewBox, bool normalized = false) const;

	void parse(const char** sInOut);

	Align		align{ALIGN_X_MID_Y_MID};
	MeetOrSlice meetOrSlice{MEET};
};

} // namespace ds::css