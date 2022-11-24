#include "stdafx.h"

#include "ds/util/string_util.h"

#include <codecvt>
#include <fstream>
#include <sstream>
#include <string>

// using namespace std;
using namespace ds;

/* WIN32
 ******************************************************************/
#ifdef WIN32
#include <iostream>
#include <time.h>
#include <windows.h>

// Disable C++17 Deprecation warning for codecvt
#pragma warning(disable : 4996)

std::wstring wstr_from_str(const std::string& str, const UINT cp) {
	if (str.empty()) return std::wstring();

	const int len = MultiByteToWideChar(cp, 0, str.c_str(), static_cast<int>(str.length()), 0, 0);
	if (!len) {
		return L"";
	}

	std::vector<wchar_t> wbuff(len + 1);
	// NOTE: this does not NULL terminate the string in wbuff, but this is ok
	//       since it was zero-initialized in the vector constructor
	if (!MultiByteToWideChar(cp, 0, str.c_str(), static_cast<int>(str.length()), &wbuff[0], len)) {
		return L"";
	}

	return &wbuff[0];
}

std::string str_from_wstr(const std::wstring& wstr, const UINT cp) {
	if (wstr.empty()) return std::string();

	const int len = WideCharToMultiByte(cp, 0, wstr.c_str(), static_cast<int>(wstr.length()), 0, 0, 0, 0);
	if (!len) {
		return "";
	}

	std::vector<char> abuff(len + 1);

	// NOTE: this does not NULL terminate the string in abuff, but this is ok
	//       since it was zero-initialized in the vector constructor
	if (!WideCharToMultiByte(cp, 0, wstr.c_str(), static_cast<int>(wstr.length()), &abuff[0], len, 0, 0)) {
		return "";
	}

	return &abuff[0];
}

namespace ds {

std::wstring wstr_from_utf8(const std::string& str) {
	return wstr_from_str(str, CP_UTF8);
}

std::string utf8_from_wstr(const std::wstring& wstr) {
	return str_from_wstr(wstr, CP_UTF8);
}


std::wstring iso_8859_1_string_to_wstring(const std::string& input) {
	// you may notice that this implementation is different than the reverse.
	// good eye.
	// wstring and string conversion is A TOTAL MOTHERFUCKER.
	// This way seems to keep some special characters, like accent marks.
	// the other way seems to throw A SILENT EXCEPTION FOR NO FUCKING REASON AND JUST SKIPS ON OUT OF YOUR CODE
	// LIKE A MOTHER FUCKER AND DOESN'T DO SHIT CAUSE WHAT THE FUCK.
	// So, yeah.
	// Here you go.
	std::wostringstream conv;
	conv << input.c_str();
	return conv.str();
}

std::string iso_8859_1_wstring_to_string(const std::wstring& input) {
	typedef std::codecvt_utf8<wchar_t>			 convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(input);
}

} // namespace ds
#else

namespace ds {
/* Linux equivalent?
 ******************************************************************/

std::wstring wstr_from_utf8(const std::string& src) {
	std::wstring dest;

	wchar_t w	  = 0;
	int		bytes = 0;
	wchar_t err	  = L'?';
	for (size_t i = 0; i < src.size(); i++) {
		unsigned char c = (unsigned char)src[i];
		if (c <= 0x7f) { // first byte
			if (bytes) {
				dest.push_back(err);
				bytes = 0;
			}
			dest.push_back((wchar_t)c);
		} else if (c <= 0xbf) { // second/third/etc byte
			if (bytes) {
				w = ((w << 6) | (c & 0x3f));
				bytes--;
				if (bytes == 0) dest.push_back(w);
			} else
				dest.push_back(err);
		} else if (c <= 0xdf) { // 2byte sequence start
			bytes = 1;
			w	  = c & 0x1f;
		} else if (c <= 0xef) { // 3byte sequence start
			bytes = 2;
			w	  = c & 0x0f;
		} else if (c <= 0xf7) { // 3byte sequence start
			bytes = 3;
			w	  = c & 0x07;
		} else {
			dest.push_back(err);
			bytes = 0;
		}
	}
	if (bytes) dest.push_back(err);


	return dest;
}


std::string utf8_from_wstr(const std::wstring& src) {
	std::string dest;

	for (size_t i = 0; i < src.size(); i++) {
		wchar_t w = src[i];
		if (w <= 0x7f)
			dest.push_back((char)w);
		else if (w <= 0x7ff) {
			dest.push_back(0xc0 | ((w >> 6) & 0x1f));
			dest.push_back(0x80 | (w & 0x3f));
		} else if (w <= 0xffff) {
			dest.push_back(0xe0 | ((w >> 12) & 0x0f));
			dest.push_back(0x80 | ((w >> 6) & 0x3f));
			dest.push_back(0x80 | (w & 0x3f));
		} else if (w <= 0x10ffff) {
			dest.push_back(0xf0 | ((w >> 18) & 0x07));
			dest.push_back(0x80 | ((w >> 12) & 0x3f));
			dest.push_back(0x80 | ((w >> 6) & 0x3f));
			dest.push_back(0x80 | (w & 0x3f));
		} else
			dest.push_back('?');
	}

	return dest;
}

} // namespace ds


#endif

namespace ds {

const float string_to_float(const std::string& str) {
	float floatValue = 0.0f;
	string_to_value<float>(str, floatValue);
	return floatValue;
}

const float wstring_to_float(const std::wstring& str) {
	float floatValue = 0.0f;
	wstring_to_value<float>(str, floatValue);
	return floatValue;
}

const int string_to_int(const std::string& str) {
	int intValue = 0;
	string_to_value<int>(str, intValue);
	return intValue;
}

const int wstring_to_int(const std::wstring& str) {
	int intValue = 0;
	wstring_to_value<int>(str, intValue);
	return intValue;
}

const double string_to_double(const std::string& str) {
	double doubleValue = 0.0;
	string_to_value<double>(str, doubleValue);
	return doubleValue;
}

const double wstring_to_double(const std::wstring& str) {
	double doubleValue = 0.0;
	wstring_to_value<double>(str, doubleValue);
	return doubleValue;
}

// variation on a function found on stackoverflow
std::vector<std::string> split(const std::string& str, const std::string& delimiters, bool dropEmpty) {
	std::size_t				 pos;
	std::size_t				 lastPos = 0;
	std::vector<std::string> splitWords;

	while (true) {
		pos = str.find(delimiters, lastPos);
		if (pos == std::string::npos) {
			pos = str.length();

			if (pos != lastPos || !dropEmpty) splitWords.push_back(std::string(str.data() + lastPos, pos - lastPos));

			break;
		} else {
			if (pos != lastPos || !dropEmpty) {
				std::string tstr = std::string(str.data() + lastPos, pos - lastPos);
				if (!tstr.empty() || !dropEmpty) splitWords.push_back(tstr);
				lastPos = pos + delimiters.size();
				continue;
			}
		}

		lastPos = pos + 1;
	}

	return splitWords;
}

std::vector<std::wstring> split(const std::wstring& str, const std::wstring& delimiters, bool dropEmpty) {
	std::size_t				  pos;
	std::size_t				  lastPos = 0;
	std::vector<std::wstring> splitWords;

	while (true) {
		pos = str.find(delimiters, lastPos);
		if (pos == std::wstring::npos) {
			pos = str.length();

			if (pos != lastPos || !dropEmpty) splitWords.push_back(std::wstring(str.data() + lastPos, pos - lastPos));

			break;
		} else {
			if (pos != lastPos || !dropEmpty) {
				splitWords.push_back(std::wstring(str.data() + lastPos, pos - lastPos /* + delimiters.size()*/));
				lastPos = pos + delimiters.size();
				continue;
			}
		}

		lastPos = pos + 1;
	}

	return splitWords;
}

std::vector<std::string> partition(const std::string& str, const std::string& partitioner) {
	std::vector<std::string> partitions;

	std::size_t pos;
	std::size_t lastPos = 0;

	while (lastPos != std::string::npos) {
		pos = str.find_first_of(partitioner, lastPos);
		if (pos != std::string::npos) {
			if (pos == lastPos) {
				partitions.push_back(std::string(str.data() + lastPos, pos - lastPos + partitioner.size()));
				lastPos += partitioner.size();
				continue;
			} else {
				partitions.push_back(std::string(str.data() + lastPos, pos - lastPos));
				partitions.push_back(partitioner);
				lastPos = pos;
				lastPos += partitioner.size();
				continue;
			}
		} else {
			pos = str.length();
			if (pos != lastPos) partitions.push_back(std::string(str.data() + lastPos, pos - lastPos));
			break;
		}
		lastPos += 1;
	}

	return partitions;
}

std::vector<std::string> partition(const std::string& str, const std::vector<std::string>& partitioners) {
	std::vector<std::string> partitions;
	partitions.push_back(str);

	for (auto it = partitioners.begin(), it2 = partitioners.end(); it != it2; ++it) {
		std::vector<std::string> tPartitions;
		for (auto itt = partitions.begin(), itt2 = partitions.end(); itt != itt2; ++itt) {
			std::vector<std::string> splitWords = ds::partition(*itt, *it);

			for (auto ittt = splitWords.begin(), ittt2 = splitWords.end(); ittt != ittt2; ++ittt) {
				tPartitions.push_back(*ittt);
			}
		}
		partitions = tPartitions;
	}

	return partitions;
}

// std::vector<std::wstring> partition( const std::wstring &str, const std::wstring &partitioner )
//{
//   std::vector<std::wstring> partitions;
//
//   std::size_t pos;
//   std::size_t lastPos = 0;
//
//   while (lastPos != std::wstring::npos) {
//     pos = str.find_first_of(partitioner, lastPos);
//     if (pos != std::wstring::npos) {
//       if (pos == lastPos) {
//         partitions.push_back(std::wstring(str.data() + lastPos, pos-lastPos+partitioner.size()));
//         lastPos += partitioner.size();
//         continue;
//       } else {
//         partitions.push_back(std::wstring(str.data() + lastPos, pos-lastPos));
//         partitions.push_back(partitioner);
//         lastPos = pos;
//         lastPos += partitioner.size();
//         continue;
//       }
//     } else {
//       pos = str.length();
//       if (pos != lastPos)
//         partitions.push_back(std::wstring(str.data() + lastPos, pos-lastPos));
//       break;
//     }
//     lastPos += 1;
//   }
//
//   return partitions;
// }

namespace {

	bool strEqual(const wchar_t* str1, const wchar_t* str2, int size) {
		for (int i = 0; i < size; ++i) {
			if (str1[i] != str2[i]) return false;
		}
		return true;
	}

} // namespace

void partition(const std::wstring& str, const std::wstring& partitioner, const Token& token,
			   std::vector<Token>& partitions) {
	int		  lastPos		  = token.pos;
	int		  partitionerSize = static_cast<int>(partitioner.size());
	const int size			  = token.pos + token.size - (partitionerSize - 1);
	for (int i = token.pos; i < size; ++i) {
		if (strEqual(&str[i], partitioner.c_str(), partitionerSize)) {
			if (i - lastPos > 0) partitions.push_back(Token(lastPos, i - lastPos));
			partitions.push_back(Token(i, partitionerSize));
			lastPos = i + partitionerSize;
		}
	}
	if (lastPos != token.pos + token.size) partitions.push_back(Token(lastPos, token.pos + token.size - lastPos));
}

std::vector<std::wstring> partition(const std::wstring& str, const std::vector<std::wstring>& partitioners) {
	// clock_t start = clock();
	std::vector<Token> partitions;
	partitions.push_back(Token(0, static_cast<int>(str.size())));

	std::vector<Token> tPartitions;
	for (auto it = partitioners.begin(), it2 = partitioners.end(); it != it2; ++it) {
		for (auto itt = partitions.begin(), itt2 = partitions.end(); itt != itt2; ++itt) {
			partition(str, *it, *itt, tPartitions);
		}
		partitions = tPartitions;
		tPartitions.clear();
	}

	std::vector<std::wstring> tokenPartitions;
	for (size_t i = 0; i < partitions.size(); ++i) {
		tokenPartitions.push_back(str.substr(partitions[i].pos, partitions[i].size));
	}

	// std::cout << "time taken: " << (double)(clock() - start) / CLOCKS_PER_SEC << std::endl;
	return tokenPartitions;
}
// std::vector<std::wstring> partition( const std::wstring &str, const std::wstring &partitioner )
//{
//   std::vector<std::wstring> partitions;
//
//   int lastPos = 0;
//   const int size = str.size() - (partitioner.size()-1);
//   for (int i = 0; i < size; ++i) {
//     if (strEqual(&str[i], partitioner.c_str(), partitioner.size())) {
//       if (i - lastPos > 0)
//         partitions.push_back(str.substr(lastPos, i - lastPos));
//       partitions.push_back(str.substr(i, partitioner.size()));
//       lastPos = i + partitioner.size();
//     }
//   }
//   if (lastPos != str.size())
//     partitions.push_back(str.substr(lastPos, str.size() - lastPos));
//
//   return partitions;
// }
//
// std::vector<std::wstring> partition( const std::wstring &str, const std::vector<std::wstring> &partitioners )
//{
//   std::vector<std::wstring> partitions;
//   partitions.push_back(str);
//
//   for (auto it = partitioners.begin(), it2 = partitioners.end(); it != it2; ++it) {
//     std::vector<std::wstring> tPartitions;
//     for (auto itt = partitions.begin(), itt2 = partitions.end(); itt != itt2; ++itt) {
//       std::vector<std::wstring> splitWords = ds::partition(*itt, *it);
//
//       for (auto ittt = splitWords.begin(), ittt2 = splitWords.end(); ittt != ittt2; ++ittt) {
//         tPartitions.push_back(*ittt);
//       }
//     }
//     partitions = tPartitions;
//   }
//
//   return partitions;
// }

int find_count(const std::string& str, const std::string& token) {
	std::size_t pos;
	std::size_t lastPos = 0;
	int			count	= 0;

	while (true) {
		pos = str.find_first_of(token, lastPos);
		if (pos == std::string::npos) {
			break;
		}

		++count;
		lastPos = pos + 1;
	}

	return count;
}

int find_count(const std::wstring& str, const std::wstring& token) {
	std::size_t pos;
	std::size_t lastPos = 0;
	int			count	= 0;

	while (true) {
		pos = str.find_first_of(token, lastPos);
		if (pos == std::wstring::npos) {
			break;
		}

		++count;
		lastPos = pos + 1;
	}

	return count;
}

void replace(std::string& str, const std::string& oldToken, const std::string& newToken) {
	std::size_t pos;
	std::size_t lastPos = 0;
	std::string tStr	= str;
	str.clear();

	while (true) {
		pos = tStr.find(oldToken, lastPos);
		if (pos == std::string::npos) {
			pos = tStr.length();

			if (pos != lastPos) {
				str += std::string(tStr.data() + lastPos, pos - lastPos);
				// str += newToken;
			}

			break;
		} else {
			str += std::string(tStr.data() + lastPos, pos - lastPos);
			str += newToken;
			lastPos = pos + oldToken.size();
			continue;
		}

		lastPos = pos + 1;
	}
}

void replace(std::wstring& str, const std::wstring& oldToken, const std::wstring& newToken) {
	std::size_t	 pos;
	std::size_t	 lastPos = 0;
	std::wstring tStr	 = str;
	str.clear();

	while (true) {
		pos = tStr.find(oldToken, lastPos);
		if (pos == std::wstring::npos) {
			pos = tStr.length();

			if (pos != lastPos) {
				str += std::wstring(tStr.data() + lastPos, pos - lastPos);
				// str += newToken;
			}

			break;
		} else {
			if (pos != lastPos) {
				str += std::wstring(tStr.data() + lastPos, pos - lastPos);
				str += newToken;
				lastPos = pos + oldToken.size();
				continue;
			}
		}

		lastPos = pos + 1;
	}
}

void loadFileIntoString(const std::string& filename, std::string& destination) {
	std::fstream filestr;
	filestr.open(filename.c_str(), std::fstream::in);
	if (filestr.is_open()) {
		filestr.seekg(0, std::fstream::end);
		unsigned count = static_cast<unsigned>(filestr.tellg());
		filestr.seekg(0, std::fstream::beg);
		char* str = new char[count];
		if (str) {
			filestr.read(str, count);
			count		= static_cast<unsigned>(filestr.gcount());
			destination = std::string(str, count);
			// destination += '\0';
			delete[] str;
		}
		filestr.close();
	}
}

void loadFileIntoString(const std::wstring& filename, std::wstring& destination) {
	std::wfstream filestr;
	filestr.open(ds::utf8_from_wstr(filename).c_str(), std::wfstream::in);
	if (filestr.is_open()) {
		filestr.seekg(0, std::wfstream::end);
		unsigned count = static_cast<unsigned>(filestr.tellg());
		filestr.seekg(0, std::wfstream::beg);
		wchar_t* str = new wchar_t[count];
		if (str) {
			filestr.read(str, count);
			count		= static_cast<unsigned>(filestr.gcount());
			destination = std::wstring(str, count);
			// destination += L'\0';
			delete[] str;
		}
		filestr.close();
	}
}

void loadFileIntoStringByLine(const std::string& filename, const std::function<void(const std::string& line)>& func) {
	if (!func) return;

	//	std::cout << "Reading file " << filename << std::endl;
	std::string f;
	ds::loadFileIntoString(filename, f);
	std::istringstream lineBuf(f);
	if (f.empty()) {
		//		std::cout << "\tERROR file does not exist or is empty" << std::endl;
		return;
	}

	while (lineBuf.good()) {
		std::string out;
		getline(lineBuf, out);
		if (!out.empty()) {
			//			std::cout << "\tsubscribe to " << out << std::endl;
			func(out);
		}
	}
}

void loadFileIntoStringByLine(const std::wstring& filename, const std::function<void(const std::wstring& line)>& func) {
	if (!func) return;

	//	std::cout << "Reading file " << filename << std::endl;
	std::wstring f;
	ds::loadFileIntoString(filename, f);
	std::wistringstream lineBuf(f);
	if (f.empty()) {
		//		std::cout << "\tERROR file does not exist or is empty" << std::endl;
		return;
	}

	while (lineBuf.good()) {
		std::wstring out;
		getline(lineBuf, out);
		if (!out.empty()) {
			//			std::cout << "\tsubscribe to " << out << std::endl;
			func(out);
		}
	}
}

void saveStringToFile(const std::string& filename, const std::string& src) {
	std::fstream filestr;
	filestr.open(filename.c_str(), std::fstream::out);
	if (filestr.is_open()) {
		filestr.write(src.c_str(), src.size());
		filestr.close();
	}
}

void saveStringToFile(const std::wstring& filename, const std::wstring& src) {
	std::wfstream filestr;
	filestr.open(ds::utf8_from_wstr(filename).c_str(), std::wfstream::out);
	if (filestr.is_open()) {
		filestr.write(src.c_str(), src.size());
		filestr.close();
	}
}

void tokenize(const std::string& input, const char delim, const std::function<void(const std::string&)>& f) {
	try {
		std::istringstream lineBuf(input);
		while (lineBuf.good()) {
			std::string out;
			getline(lineBuf, out, delim);
			if (f) f(out);
		}
	} catch (std::exception&) {}
}

void to_lowercase(std::wstring& str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void to_lowercase(std::string& str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void to_uppercase(std::string& str) {
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

void to_uppercase(std::wstring& str) {
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

ci::vec3 parseVector(const std::string& s) {
	auto	 tokens = ds::split(s, ", ", true);
	ci::vec3 v;
	v.x = tokens.size() > 0 ? ds::string_to_float(tokens[0]) : 0.0f;
	v.y = tokens.size() > 1 ? ds::string_to_float(tokens[1]) : 0.0f;
	v.z = tokens.size() > 2 ? ds::string_to_float(tokens[2]) : 0.0f;

	return v;
}

ci::vec4 parseVector4(const std::string& s) {
	auto	 tokens = ds::split(s, ", ", true);
	ci::vec4 v;
	v.x = tokens.size() > 0 ? ds::string_to_float(tokens[0]) : 0.0f;
	v.y = tokens.size() > 1 ? ds::string_to_float(tokens[1]) : 0.0f;
	v.z = tokens.size() > 2 ? ds::string_to_float(tokens[2]) : 0.0f;
	v.w = tokens.size() > 3 ? ds::string_to_float(tokens[3]) : 0.0f;

	return v;
}

ci::Rectf parseRect(const std::string& s) {
	auto	  tokens = ds::split(s, ", ", true);
	ci::Rectf v;
	v.x1 = tokens.size() > 0 ? ds::string_to_float(tokens[0]) : 0.0f;
	v.y1 = tokens.size() > 1 ? ds::string_to_float(tokens[1]) : 0.0f;
	v.x2 = tokens.size() > 2 ? ds::string_to_float(tokens[2]) : 0.0f;
	v.y2 = tokens.size() > 3 ? ds::string_to_float(tokens[3]) : 0.0f;

	v.x2 += v.x1;
	v.y2 += v.y1;

	return v;
}

std::string unparseRect(const ci::Rectf& v) {
	std::stringstream ss;
	ss << v.x1 << ", " << v.y1 << ", " << v.getWidth() << ", " << v.getHeight();
	return ss.str();
}

std::string unparseVector(const ci::vec2& v) {
	std::stringstream ss;
	ss << v.x << ", " << v.y;
	return ss.str();
}

std::string unparseVector(const ci::vec3& v) {
	std::stringstream ss;
	ss << v.x << ", " << v.y << ", " << v.z;
	return ss.str();
}

std::string unparseVector(const ci::vec4& v) {
	std::stringstream ss;
	ss << v.x << ", " << v.y << ", " << v.z << ", " << v.w;
	return ss.str();
}

bool parseBoolean(const std::string& s) {
	return (s == "true" || s == "TRUE" || s == "yes" || s == "YES" || s == "on" || s == "ON" || s == "1") ? true
																										  : false;
}

std::string unparseBoolean(const bool b) {
	if (b) return "true";
	return "false";
}

void tokenize(const std::string& input, const std::function<void(const std::string&)>& f) {
	tokenize(input, '\n', f);
}

std::vector<std::pair<int, std::string>> extractPairs(const std::string& value, const std::string& leftDelim,
													  const std::string& rightDelim) {
	std::vector<std::pair<int, std::string>> output;

	const auto ldLen = leftDelim.length();
	const auto rdLen = rightDelim.length();

	auto   leftMatch  = value.find(leftDelim);
	auto   rightMatch = value.find(rightDelim, leftMatch);
	size_t lastMatch  = 0;

	while (leftMatch != std::string::npos && rightMatch != std::string::npos) {
		if (leftMatch != lastMatch) {
			std::string thing = value.substr(lastMatch, leftMatch - lastMatch);
			output.push_back(std::make_pair(0, thing));
		}

		std::string thing = value.substr(leftMatch + ldLen, rightMatch - (leftMatch + ldLen));
		output.push_back(std::make_pair(1, thing));


		lastMatch  = rightMatch + rdLen;
		leftMatch  = value.find(leftDelim, rightMatch);
		rightMatch = value.find(rightDelim, leftMatch);
	}

	if (lastMatch < value.length()) {
		std::string thing = value.substr(lastMatch);
		output.push_back(std::make_pair(0, thing));
	}

	return output;
}

} // namespace ds
