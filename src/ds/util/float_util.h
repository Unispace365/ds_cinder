#pragma once
#ifndef DS_UTIL_FLOATUTIL_H_
#define DS_UTIL_FLOATUTIL_H_

namespace ds {

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

} // namespace ds

#endif