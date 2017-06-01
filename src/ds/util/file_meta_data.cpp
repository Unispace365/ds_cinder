#include "stdafx.h"

#include "file_meta_data.h"

#include <Poco/Path.h>
#include <Poco/File.h>
#include <boost/filesystem.hpp>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>

namespace ds {

namespace {
const std::string		EMPTY_SZ("");
}

/**
 * \class ds::FileMetaData
 */
FileMetaData::FileMetaData() {
}

FileMetaData::FileMetaData(const std::string& filename) {
	parse(filename);
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


bool safeFileExistsCheck(const std::string filePath, const bool allowDirectory){
	Poco::File xmlFile(filePath);
	bool fileExists = false;
	try{
		if(xmlFile.exists()){
			fileExists = true;
		}
		if(fileExists && !allowDirectory){
			if(xmlFile.isDirectory()) fileExists = false;
		}
	} catch(std::exception&){}

	return fileExists;
}

std::string filePathRelativeTo(const std::string &base, const std::string &relative){
	if(relative.find("%APP%") != std::string::npos){
		return ds::Environment::expand(relative);
	}

	using namespace boost::filesystem;
	boost::system::error_code e;
	std::string ret = canonical(path(relative), path(base).parent_path(), e).string();
	if(e.value() != boost::system::errc::success) {
		DS_LOG_WARNING("Trying to use bad relative file path: " << relative << ": " << e.message());
	}
	return ret;
}

std::string	getNormalizedPath(const std::string& path) {
	auto ret = path;
	std::replace(ret.begin(), ret.end(), '\\', '/');

	ret = Poco::Path(ret).toString();

	return ret;
}

std::string	getNormalizedPath(const Poco::Path& path) {
	return getNormalizedPath(path.toString());
}

} // namespace ds
