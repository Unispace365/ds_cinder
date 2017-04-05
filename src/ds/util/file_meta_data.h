#pragma once
#ifndef DS_UTIL_FILEMETADATA_H_
#define DS_UTIL_FILEMETADATA_H_

#include <exception>
#include <string>
#include <vector>
#include <cinder/Vector.h>
#include <Poco/Path.h>
#include <ds/util/string_util.h>

namespace ds {

/// Gets the absolute file path given a base file path a relative path to that base.
std::string		filePathRelativeTo(const std::string &base, const std::string &relative); 

/// Poco throws an exception when calling file.exists() and the file doesn't exist, so this handles that for you with no exceptions thrown.
bool			safeFileExistsCheck(const std::string filePath, const bool allowDirectory = true);

/// Return a platform-native normalized path string
std::string		getNormalizedPath(const std::string& path);
std::string		getNormalizedPath(const Poco::Path& path);


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

} // namespace ds

#endif // DS_UTIL_FILEMETADATA_H_
