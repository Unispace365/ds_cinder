#include "file_name_parser.h"

#include <map>
#include <Poco/Path.h>

namespace ds {

namespace {
const std::string		EMPTY_SZ("");
}

/**
 * \class ds::ParseFileMetaException
 */
ParseFileMetaException::ParseFileMetaException(const std::string& metaValue)
	: mWhat(metaValue) {
}

const char *ParseFileMetaException::what() const throw() {
	return mWhat.c_str();
}

/**
 * \class ds::FileMetaData
 */
FileMetaData::FileMetaData() {
}

void FileMetaData::parse(const std::string& filename) {
	mAttrib.clear();

	Poco::Path			p(filename);
	std::vector<std::string> splitString = ds::split(p.getBaseName(), ".");
	for (auto it = splitString.begin(), it2 = splitString.end(); it != it2; ++it ) {
		// First item is base name
		if (it == splitString.begin()) continue;
		std::string		str = *it;
		const size_t	pos = str.find("_");
		if (pos == std::string::npos) continue;

		std::vector<std::string> meta = split(str, "_");
		if (meta.size() != 2) continue;
		mAttrib.push_back(std::pair<std::string, std::string>(meta.front(), meta.back()));
	}
}

size_t FileMetaData::keyCount() const {
	return mAttrib.size();
}

bool FileMetaData::contains(const std::string& key) const {
	for (auto it=mAttrib.begin(), end=mAttrib.end(); it!=end; ++it) {
		if (it->first == key) return true;
	}
	return false;
}

const std::string& FileMetaData::keyAt(const size_t index) const {
	if (index >= mAttrib.size()) return EMPTY_SZ;
	return mAttrib[index].first;
}

const std::string& FileMetaData::valueAt(const size_t index) const {
	if (index >= mAttrib.size()) return EMPTY_SZ;
	return mAttrib[index].second;
}

const std::string& FileMetaData::findValue(const std::string& key) const {
	for (auto it=mAttrib.begin(), end=mAttrib.end(); it!=end; ++it) {
		if (it->first == key) return it->second;
	}
	return EMPTY_SZ;
}

int parseFileMetaData( const std::string &filename, const std::string &metaValue ) {
  std::vector<std::string> splitString = ds::split(filename, ".");

  for ( auto it = splitString.begin(), it2 = splitString.end(); it != it2; ++it ) {
  	std::string str = *it;
    size_t pos = str.find(metaValue);
    if (pos != std::string::npos && pos == 0) {
      std::vector<std::string> splitMeta = split(str, "_");
      int val;
      if (!string_to_value(splitMeta.back(), val))
        throw ParseFileMetaException("Was unable to find meta data for: "+metaValue);
      return val;
    }
  }

  throw ParseFileMetaException("Was unable to find meta data for: "+metaValue);
}

static std::map<std::string, ci::Vec2f> ParsedAttribs;

ci::Vec2f parseFileMetaDataSize(const std::string &filename) {
	std::map<std::string, ci::Vec2f>::iterator found = ParsedAttribs.find(filename);

	if (found != ParsedAttribs.end())
		return found->second;

	int wVal = parseFileMetaData(filename, "w_");
	int hVal = parseFileMetaData(filename, "h_");

	ci::Vec2f values(static_cast<float>(wVal), static_cast<float>(hVal));
	ParsedAttribs[filename] = values;

	return values;
}

void getFileMetaData(const std::string& filename, std::vector<std::pair<std::string, std::string>>& out) {
	
}

}