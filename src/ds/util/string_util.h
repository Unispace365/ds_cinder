#pragma once
#ifndef DS_UTIL_STRINGUTIL_H_
#define DS_UTIL_STRINGUTIL_H_

#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace ds {

class conversion_error : public std::exception { };

// Format conversions
std::wstring		wstr_from_utf8(const std::string&);		// throws conversion_error
std::string			utf8_from_wstr(const std::wstring&);	// throws conversion_error

// doesn't throw anything, but uses c++11 fanciness. Keeps some characters (like letters with accents) better
std::wstring		string_to_wstring(const std::string& input);
std::string			wstring_to_string(const std::wstring& input);

// Number conversions
template <typename T>
bool wstring_to_value(const std::wstring& str, T& ans)
{
	std::wistringstream		ss(str);
	T						v;
	ss >> v;
	if (ss.eof() && !ss.fail()) { 
		ans = v;
		return true;
	}
	return false;
}

template <typename T>
bool string_to_value(const std::string& str, T& ans)
{
	std::istringstream		ss(str);
	T						v;
	ss >> v;
	if (ss.eof() && !ss.fail()) { 
		ans = v;
		return true;
	}
	return false;
}

template <typename T>
std::string value_to_string(T number)
{
	std::ostringstream ss;
	ss << number;
	return ss.str();
}

template <typename T>
std::wstring value_to_wstring(T number)
{
	std::wostringstream ss;
	ss << number;
	return ss.str();
}

std::vector<std::string> split( const std::string &str, const std::string &delimiters, bool dropEmpty = false );
std::vector<std::wstring> split( const std::wstring &str, const std::wstring &delimiters, bool dropEmpty = false );

struct Token
{
  Token(int pos = 0, int size = 0)
    : pos(pos)
    , size(size) {}
  int pos;
  int size;
};

std::vector<std::string> partition( const std::string &str, const std::string &partitioner );
std::vector<std::string> partition(const std::string &str, const std::vector<std::string> &partitioners);
void partition( const std::wstring &str, const std::wstring &partitioner, const Token &token, std::vector<Token> &partitions );
std::vector<std::wstring> partition(const std::wstring &str, const std::vector<std::wstring> &partitioners);

int find_count( const std::string &str, const std::string &token );
int find_count( const std::wstring &str, const std::wstring &token );

void replace( std::string &str, const std::string &oldToken, const std::string &newToken );
void replace( std::wstring &str, const std::wstring &oldToken, const std::wstring &newToken );

void loadFileIntoString( const std::string &filename, std::string &destination );
void loadFileIntoString( const std::wstring &filename, std::wstring &destination );
// Same as above but send each line to the function
void loadFileIntoStringByLine( const std::string &filename, const std::function<void(const std::string& line)>& );
void loadFileIntoStringByLine( const std::wstring &filename, const std::function<void(const std::wstring& line)>& );

void saveStringToFile( const std::string &filename, const std::string &src );
void saveStringToFile( const std::wstring &filename, const std::wstring &src );

// Tokenize the input, passing each token to the supplied function
void tokenize(const std::string& input, const char delim, const std::function<void(const std::string&)>&);
void tokenize(const std::string& input, const std::function<void(const std::string&)>&);

} // namespace ds

#endif // DS_UTIL_STRINGUTIL_H_
