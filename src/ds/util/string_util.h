#pragma once
#ifndef DS_UTIL_STRINGUTIL_H_
#define DS_UTIL_STRINGUTIL_H_

#include <cinder/Rect.h>
#include <cinder/Vector.h>
#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace ds {
// Format conversions
std::wstring wstr_from_utf8(const std::string&);
std::string	 utf8_from_wstr(const std::wstring&);

// If you have some ANSI text, iso 8859-1
std::wstring iso_8859_1_string_to_wstring(const std::string& input);
std::string	 iso_8859_1_wstring_to_string(const std::wstring& input);

// Number conversions
template <typename T>
bool wstring_to_value(const std::wstring& str, T& ans) {
	std::wistringstream ss(str);
	T					v;
	ss >> v;
	if (ss.eof() && !ss.fail()) {
		ans = v;
		return true;
	}
	return false;
}

template <typename T>
bool string_to_value(const std::string& str, T& ans) {
	std::istringstream ss(str);
	T				   v;
	ss >> v;
	if (ss.eof() && !ss.fail()) {
		ans = v;
		return true;
	}
	return false;
}

template <typename T>
std::string value_to_string(T number) {
	std::ostringstream ss;
	ss << number;
	return ss.str();
}

template <typename T>
std::wstring value_to_wstring(T number) {
	std::wostringstream ss;
	ss << number;
	return ss.str();
}

// extra convenience, so you can use inline
const float	 string_to_float(const std::string& str);
const float	 wstring_to_float(const std::wstring& str);
const int	 string_to_int(const std::string& str);
const int	 wstring_to_int(const std::wstring& str);
const double string_to_double(const std::string& str);
const double wstring_to_double(const std::wstring& str);

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6) {
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return out.str();
}

std::vector<std::string>  split(const std::string& str, const std::string& delimiters, bool dropEmpty = false);
std::vector<std::wstring> split(const std::wstring& str, const std::wstring& delimiters, bool dropEmpty = false);

struct Token {
	Token(int pos = 0, int size = 0)
	  : pos(pos)
	  , size(size) {}
	int pos;
	int size;
};

std::vector<std::string>  partition(const std::string& str, const std::string& partitioner);
std::vector<std::string>  partition(const std::string& str, const std::vector<std::string>& partitioners);
void					  partition(const std::wstring& str, const std::wstring& partitioner, const Token& token,
									std::vector<Token>& partitions);
std::vector<std::wstring> partition(const std::wstring& str, const std::vector<std::wstring>& partitioners);

int find_count(const std::string& str, const std::string& token);
int find_count(const std::wstring& str, const std::wstring& token);

void replace(std::string& str, const std::string& oldToken, const std::string& newToken);
void replace(std::wstring& str, const std::wstring& oldToken, const std::wstring& newToken);

void loadFileIntoString(const std::string& filename, std::string& destination);
void loadFileIntoString(const std::wstring& filename, std::wstring& destination);
// Same as above but send each line to the function
void loadFileIntoStringByLine(const std::string& filename, const std::function<void(const std::string& line)>&);
void loadFileIntoStringByLine(const std::wstring& filename, const std::function<void(const std::wstring& line)>&);

void saveStringToFile(const std::string& filename, const std::string& src);
void saveStringToFile(const std::wstring& filename, const std::wstring& src);

// Tokenize the input, passing each token to the supplied function
void tokenize(const std::string& input, const char delim, const std::function<void(const std::string&)>&);
void tokenize(const std::string& input, const std::function<void(const std::string&)>&);

void to_lowercase(std::string& str);
void to_lowercase(std::wstring& str);
void to_uppercase(std::string& str);
void to_uppercase(std::wstring& str);

/// Parses a string into a 3d vector. Example: size="400, 400, 0" the space after the comma is required to read the
/// second and third token. Defaults parameters to 0 if they don't exist.
ci::vec3 parseVector(const std::string& s);

/// Parses a string into a 4d vector. Example: size="400, 400, 0" the space after the comma is required to read the
/// second and third token. Defaults parameters to 0 if they don't exist.
ci::vec4 parseVector4(const std::string& s);

/// Parses a string into a rectangle. Example: size="400, 400, 0, 0", where it's "L, T, W, H" the space after the comma
/// is required to read the second and third token. Defaults parameters to 0 if they don't exist.
ci::Rectf parseRect(const std::string& s);

/// The inverse of parseRect. For an input of ci::Rectf(100.0f, 100.0f, 2220.0f, 1180.0f) returns "100.0, 100.0, 1920.0,
/// 1080.0"
///														X1		Y1		X2		 Y2				   L	  T		 W H
std::string unparseRect(const ci::Rectf& v);

/// The inverse of parseVector. For an input of ci::vec2(123.0f, 0.0f) returns "123.0, 0.0"
std::string unparseVector(const ci::vec2& v);

/// The inverse of parseVector. For an input of ci::vec3(123.0f, 0.0f, 987.6f) returns "123.0, 0.0, 987.6"
std::string unparseVector(const ci::vec3& v);

/// The inverse of parseVector. For an input of ci::vec4(123.0f, 0.0f, 987.6f, 42.1f) returns "123.0, 0.0, 987.6, 42.1"
std::string unparseVector(const ci::vec4& v);


/// Parse true/false from a string.
bool parseBoolean(const std::string& s);

/// The inverse of parseBoolean
std::string unparseBoolean(const bool b);

std::vector<std::pair<int, std::string>> extractPairs(const std::string& value, const std::string& leftDelim,
													  const std::string& rightDelim);


/// Parser functions.

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
/// Skips any of the \a chars in the ASCII string.
inline void skipAnyOf(const char** sInOut, const std::string &chars) {
	while (**sInOut && chars.find(**sInOut) != std::string::npos)
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

#endif // DS_UTIL_STRINGUTIL_H_
