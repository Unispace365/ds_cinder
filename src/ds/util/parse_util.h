#pragma once
#ifndef DS_UTIL_PARSEUTIL_H_
#define DS_UTIL_PARSEUTIL_H_

#include <string>

namespace ds {

/// Returns whether \a c is a digit ASCII character.
inline bool isDigit(char c) {
	return (c >= '0' && c <= '9');
}
/// Returns whether \a c is a numeric ASCII character, including '.', '-', 'e', 'E', and '+'.
inline bool isNumeric(char c) {
	return (c >= '0' && c <= '9') || c == '.' || c == '-' || c == 'e' || c == 'E' || c == '+';
}
/// Skips any whitespace in the ASCII string.
inline void skipSpace(const char** sInOut) {
	while (**sInOut && isspace(**sInOut))
		++(*sInOut);
}
/// Skips any whitespace or commas in the ASCII string.
inline void skipSpaceOrComma(const char** sInOut) {
	while (**sInOut && (isspace(**sInOut) || **sInOut == ','))
		++(*sInOut);
}
/// Skips any whitespace or parenthesis in the ASCII string.
inline void skipSpaceOrParenthesis(const char** sInOut) {
	while (**sInOut && (isspace(**sInOut) || **sInOut == '(' || **sInOut == ')'))
		++(*sInOut);
}
/// Skips all characters until the given character is found in the ASCII string.
inline void skipUntil(const char** sInOut, char c) {
	while (**sInOut && **sInOut != c)
		++(*sInOut);
}
/// Returns the part before the first occurrence of the given character in the ASCII string.
inline std::string fetchUntil(const char** sInOut, char c) {
	const char* from = *sInOut;
	skipUntil(sInOut, c);
	return std::string(from, *sInOut - from);
}
/// Returns the first word in the ASCII string.
inline std::string fetchWord(const char** sInOut) {
	const char* from = *sInOut;
	while (**sInOut && !isspace(**sInOut))
		++(*sInOut);
	return std::string(from, *sInOut - from);
}
/// Returns the double value at the beginning of the ASCII string.
inline double parseDouble(const char** sInOut) {
	return strtod(*sInOut, const_cast<char**>(sInOut));
}
/// Returns the float point value at the beginning of the ASCII string.
inline float parseFloat(const char** sInOut) {
	return strtof(*sInOut, const_cast<char**>(sInOut));
}
/// Returns the integer value at the beginning of the ASCII string.
inline long int parseInt(const char** sInOut) {
	return strtol(*sInOut, const_cast<char**>(sInOut), 10);
}
/// Returns the integer value of the hexadecimal digits at the beginning of the ASCII string.
inline long int parseHex(const char** sInOut) {
		return strtol(*sInOut, const_cast<char**>(sInOut), 16);
}
/// Returns the integer value of the binary digits at the beginning of the ASCII string.
inline long int parseBinary(const char** sInOut) {
	return strtol(*sInOut, const_cast<char**>(sInOut), 2);
}
/// Returns whether the ASCII strings are equal, ignoring case.
inline bool isSimilar(const std::string& a, const std::string& b) {
	return a.size() == b.size() &&
		   std::equal(a.begin(), a.end(), b.begin(), [](char a, char b) { return tolower(a) == tolower(b); });
}
/// Returns whether the ASCII strings are equal, ignoring case.
inline bool isSimilar(const char* a, const char* b, size_t maxCount) {
	return std::equal(a, a + maxCount, b, b + maxCount, [](char a, char b) { return tolower(a) == tolower(b); });
}

} // namespace ds

#endif // DS_UTIL_PARSEUTIL_H_