#pragma once
#ifndef DS_UTIL_FILE_NAME_PARSER_H
#define DS_UTIL_FILE_NAME_PARSER_H

#include <exception>
#include <string>
#include <vector>
#include <cinder/Vector.h>
#include <ds/util/string_util.h>


namespace ds {

/**
 * \class ds::ParseFileMetaException
 */
class ParseFileMetaException : public std::exception {
public:
	ParseFileMetaException(const std::string& metaValue);
	const char *what() const throw();

private:
	std::string mWhat;
};

/**
 * \class ds::FileMetaData
 * Collection of file meta data.
 */
class FileMetaData {
public:
	FileMetaData();
	explicit FileMetaData(const std::string& filename);

	void				parse(const std::string& filename);

	size_t				keyCount() const;
	bool				contains(const std::string& key) const;
	const std::string&	keyAt(const size_t index) const;
	const std::string&	valueAt(const size_t index) const;
	const std::string&	findValue(const std::string& key) const;
	template <typename T>
	T					findValueType(const std::string& key, const T error) const;

private:
	std::vector<std::pair<std::string, std::string>>
						mAttrib;
};

template <typename T>
T FileMetaData::findValueType(const std::string& key, const T error) const {
	const std::string&	v = findValue(key);
	if (v.empty()) return error;
	T					ans;
	if (!ds::string_to_value(v, ans)) return error;
	return ans;
}

}

#endif//DS_UTIL_FILE_NAME_PARSER_H
