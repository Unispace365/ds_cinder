#include "ds/data/resource.h"

#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <Poco/Path.h>
#include <Poco/File.h>
#include "ds/app/environment.h"
#include "ds/data/data_buffer.h"
#include "ds/debug/debug_defines.h"
#include "ds/debug/logger.h"
#include "ds/query/query_client.h"
#include "ds/util/image_meta_data.h"
#include "ds/util/file_meta_data.h"

namespace {
const std::string		FONT_TYPE_SZ("f");
const std::string		IMAGE_TYPE_SZ("i");
const std::string		IMAGE_SEQUENCE_TYPE_SZ("s");
const std::string		PDF_TYPE_SZ("p");
const std::string		VIDEO_TYPE_SZ("v");
const std::string		VIDEO_STREAM_TYPE_SZ("vs");
const std::string		WEB_TYPE_SZ("w");

// This gets reduced to being a video type; here to support B&R CMSs, which
// can't have audio files that are typed as video.
const std::string		AUDIO_TYPE_SZ("a");

const std::string		EMPTY_SZ("");

const std::wstring		FONT_NAME_SZ(L"font");
const std::wstring		IMAGE_NAME_SZ(L"image");
const std::wstring		IMAGE_SEQUENCE_NAME_SZ(L"image sequence");
const std::wstring		PDF_NAME_SZ(L"pdf");
const std::wstring		VIDEO_NAME_SZ(L"video");
const std::wstring		VIDEO_STREAM_NAME_SZ(L"video stream");
const std::wstring		WEB_NAME_SZ(L"web");
const std::wstring		ERROR_NAME_SZ(L"error");
}

namespace ds {

/**
 * ds::Resource::Id
 */
Resource::Id::Id()
	: mType(CMS_TYPE)
	, mValue(0)
{
}

Resource::Id::Id(const int value)
	: mType(CMS_TYPE)
	, mValue(value)
{
}

Resource::Id::Id(const char type, const int value)
	: mType(type)
	, mValue(value)
{
}

bool Resource::Id::operator==(const Id& o) const
{
	return mType == o.mType && mValue == o.mValue;
}

bool Resource::Id::operator!=(const Id& o) const
{
	return mType != o.mType || mValue != o.mValue;
}

bool Resource::Id::operator>(const Id& o) const
{
	if (mType == o.mType) return mValue > o.mValue;
	return mType > o.mType;
}

bool Resource::Id::operator<(const Id& o) const
{
	if (mType == o.mType) return mValue < o.mValue;
	return mType < o.mType;
}

bool Resource::Id::operator>(const int value) const
{
	return mValue > value;
}

bool Resource::Id::operator>=(const int value) const
{
	return mValue >= value;
}

bool Resource::Id::operator<(const int value) const
{
	return mValue < value;
}

bool Resource::Id::operator<=(const int value) const
{
	return mValue <= value;
}

bool Resource::Id::empty() const
{
	// Database IDs must start with 1, so that's our notion of valid.
	return mValue < 1;
}

void Resource::Id::clear()
{
	*this = Id();
}

void Resource::Id::swap(Id& id)
{
	std::swap(mType, id.mType);
	std::swap(mValue, id.mValue);
}

static bool try_parse(const std::string& s, const std::string& typeStr, const char type, Resource::Id& out)
{
	if (s.compare(0, typeStr.size(), typeStr) != 0) return false;
	if (s.length() <= typeStr.size()) return false;

	out.mType = type;
	out.mValue = atoi(s.c_str()+typeStr.size());
	return true;
}

bool Resource::Id::tryParse(const std::string& s)
{
	static const char				TYPE_A[] = { CMS_TYPE, APP_TYPE };
	static const std::string		TAG_A[] = { "cms:", "app:" };
	for (int k=0; k<2; ++k) {
		Id							v;
		if (try_parse(s, TAG_A[k], TYPE_A[k], v)) {
			*this = v;
			return true;
		}
	}
	DS_DBG_CODE(std::cout << "ERROR ds::resource_id::tryParse() illegal input (" << s << ")" << std::endl);
	return false;
}

bool Resource::Id::verifyPaths() const
{
	if (getResourcePath().empty()) {
//		DS_LOG_ERROR_M("resource_id (" << *this << ") missing resource path", ds::GENERAL_LOG);
		return false;
	}
	if (getDatabasePath().empty()) {
//		DS_LOG_ERROR_M("resource_id (" << *this << ") missing database path", ds::GENERAL_LOG);
		return false;
	}
	return true;
}

void Resource::Id::writeTo(DataBuffer& buf) const
{
	buf.add(mType);
	buf.add(mValue);
}

bool Resource::Id::readFrom(DataBuffer& buf)
{
	if (!buf.canRead<char>()) return false;
	mType = buf.read<char>();
	if (!buf.canRead<int>()) return false;
	mValue = buf.read<int>();
	return true;
}

/**
 * ds::Resource::id database path
 */
namespace {
std::string				CMS_RESOURCE_PATH("");
std::string				CMS_PORTABLE_RESOURCE_PATH("");
std::string				CMS_DB_PATH("");
std::string				APP_RESOURCE_PATH("");
std::string				APP_DB_PATH("");
const std::string		EMPTY_PATH("");
// Function for generating custom paths
std::function<const std::string&(const Resource::Id&)>
						CUSTOM_RESOURCE_PATH;
std::function<const std::string&(const Resource::Id&)>
						CUSTOM_DB_PATH;
}

const std::string& Resource::Id::getResourcePath() const
{
	if (mType == CMS_TYPE)		return CMS_RESOURCE_PATH;
	if (mType == APP_TYPE)		return APP_RESOURCE_PATH;
	if (mType <= CUSTOM_TYPE && CUSTOM_RESOURCE_PATH) return CUSTOM_RESOURCE_PATH(*this);
	return EMPTY_PATH;
}

const std::string& Resource::Id::getDatabasePath() const
{
	if (mType == CMS_TYPE)		return CMS_DB_PATH;
	if (mType == APP_TYPE)		return APP_DB_PATH;
	if (mType <= CUSTOM_TYPE && CUSTOM_DB_PATH) return CUSTOM_DB_PATH(*this);
	return EMPTY_PATH;
}

const std::string& Resource::Id::getPortableResourcePath() const {
	if (mType == CMS_TYPE)		return CMS_PORTABLE_RESOURCE_PATH;
	return EMPTY_PATH;
}

void Resource::Id::setupPaths(const std::string& resource, const std::string& db,
							  const std::string& projectPath)
{
	CMS_RESOURCE_PATH = resource;
	{
		Poco::Path      p(resource);
		p.append(db);
		CMS_DB_PATH = p.toString();
	}

	// Portable path. We want it as small as possible to ease network traffic.
	std::string			local = ds::Environment::expand("%LOCAL%");
	if (boost::starts_with(resource, local)) {
		CMS_PORTABLE_RESOURCE_PATH = "%LOCAL%" + resource.substr(local.size());
	} else {
		DS_LOG_ERROR("CMS resource path (" << CMS_RESOURCE_PATH << ") does not start with %LOCAL% (" << local << ")");
		CMS_PORTABLE_RESOURCE_PATH = resource;
	}

	// If the project path exists, then setup our app-local resources path.
	if (!projectPath.empty()) {
		Poco::Path			p(Environment::getDownstreamDocumentsFolder());
		p.append("resources");
		p.append(projectPath);
		p.append("app");
		APP_RESOURCE_PATH = p.toString();

		p.append("db");
		p.append("db.sqlite");
		APP_DB_PATH = p.toString();

		// Make sure we have the trailing separator on the resource path.
		if (!APP_RESOURCE_PATH.empty() && APP_RESOURCE_PATH.back() != Poco::Path::separator()) {
			APP_RESOURCE_PATH.append(1, Poco::Path::separator());
		}
	}

	DS_LOG_INFO("CMS_RESOURCE_PATH: " << CMS_RESOURCE_PATH);
	DS_LOG_INFO("CMS_PORTABLE_RESOURCE_PATH: " << CMS_PORTABLE_RESOURCE_PATH);
	DS_LOG_INFO("APP_RESOURCE_PATH: " << APP_RESOURCE_PATH);
	
}

void Resource::Id::setupCustomPaths( const std::function<const std::string&(const Resource::Id&)>& resourcePath,
									 const std::function<const std::string&(const Resource::Id&)>& dbPath)
{
	CUSTOM_RESOURCE_PATH = resourcePath;
	CUSTOM_DB_PATH = dbPath;
}

/**
 * ds::Resource
 */
Resource Resource::fromImage(const std::string& full_path) {
	Resource		r;
	r.mType = IMAGE_TYPE;
	r.mLocalFilePath = full_path;
	ImageMetaData	meta(full_path);
	r.mWidth = meta.mSize.x;
	r.mHeight = meta.mSize.y;
	return r;
}

Resource Resource::fromQuery(const Resource::Id& id) {
	Resource		r;
	if (id.mValue < 1) return r;
	r.query(id);
	return r;
}

Resource::Resource()
	: mType(ERROR_TYPE)
	, mDuration(0)
	, mWidth(0)
	, mHeight(0)
	, mThumbnailId(0)
	, mParentId(0)
	, mParentIndex(0)
{
}

Resource::Resource(const Resource::Id& dbId, const int type)
	: mDbId(dbId)
	, mType(type)
	, mDuration(0)
	, mWidth(0)
	, mHeight(0)
	, mThumbnailId(0)
	, mParentId(0)
	, mParentIndex(0)
{
}
Resource::Resource(const std::string& fullPath)
	: mDbId(0)
	, mType(parseTypeFromFilename(fullPath))
	, mDuration(0)
	, mWidth(0)
	, mHeight(0)
	, mThumbnailId(0)
	, mParentId(0)
	, mParentIndex(0)
	, mLocalFilePath(fullPath)
{
}

Resource::Resource(const std::string& fullPath, const int type)
	: mDbId(0)
	, mType(type)
	, mDuration(0)
	, mWidth(0)
	, mHeight(0)
	, mThumbnailId(0)
	, mParentId(0)
	, mParentIndex(0)
	, mLocalFilePath(fullPath)
{
}

Resource::Resource(const std::string& localFullPath, const float width, const float height)
	: mDbId(0)
	, mType(parseTypeFromFilename(localFullPath))
	, mDuration(0)
	, mWidth(width)
	, mHeight(height)
	, mThumbnailId(0)
	, mParentId(0)
	, mParentIndex(0)
	, mLocalFilePath(localFullPath){
}

Resource::Resource(const Resource::Id dbid, const int type, const double duration, 
				   const float width, const float height, const std::string filename, 
				   const std::string path, const int thumbnailId, const std::string fullFilePath)
	: mDbId(dbid)
	, mType(type)
	, mDuration(duration)
	, mWidth(width)
	, mHeight(height)
	, mFileName(filename)
	, mPath(path)
	, mThumbnailId(thumbnailId)
	, mParentId(0)
	, mParentIndex(0)
	, mLocalFilePath(fullFilePath)
{}
	

bool Resource::operator==(const Resource& o) const {
	if(mLocalFilePath != o.mLocalFilePath) return false;
	return mDbId == o.mDbId && mType == o.mType && mDuration == o.mDuration && mWidth == o.mWidth && mHeight == o.mHeight && mFileName == o.mFileName && mPath == o.mPath;
}

bool Resource::operator!=(const Resource& o) const {
	return (!(*this == o));
}

const std::wstring& Resource::getTypeName() const {
	if (mType == FONT_TYPE) return FONT_NAME_SZ;
	else if (mType == IMAGE_TYPE) return IMAGE_NAME_SZ;
	else if (mType == IMAGE_SEQUENCE_TYPE) return IMAGE_SEQUENCE_NAME_SZ;
	else if (mType == PDF_TYPE) return PDF_NAME_SZ;
	else if (mType == VIDEO_TYPE) return VIDEO_NAME_SZ;
	else if(mType == VIDEO_STREAM_TYPE) return VIDEO_STREAM_NAME_SZ;
	else if (mType == WEB_TYPE) return WEB_NAME_SZ;
	return ERROR_NAME_SZ;
}

std::string Resource::getAbsoluteFilePath() const {
	if(!mLocalFilePath.empty()) return mLocalFilePath;

	if (mFileName.empty()) return EMPTY_SZ;
	if (mType == WEB_TYPE) return mFileName;
	Poco::Path        p(mDbId.getResourcePath());
	if (p.depth() < 1) return EMPTY_SZ;
	p.append(mPath).append(mFileName);
	return p.toString();
}

std::string Resource::getPortableFilePath() const {
	if(!mLocalFilePath.empty()) return mLocalFilePath;

	if (mFileName.empty()) return EMPTY_SZ;
	if (mType == WEB_TYPE) return mFileName;
	Poco::Path        p(mDbId.getPortableResourcePath());
	if (p.depth() < 1) return EMPTY_SZ;
	p.append(mPath).append(mFileName);
	return p.toString();
}

void Resource::clear() {
	mDbId.clear();
	mType = ERROR_TYPE;
	mDuration = 0.0;
	mWidth = 0.0f;
	mHeight = 0.0f;
	mFileName.clear();
	mPath.clear();
	mThumbnailId = 0;
	mLocalFilePath.clear();
}

bool Resource::empty() const {
	if(!mLocalFilePath.empty()) return false;
	if(!mFileName.empty()) return false;
	return mDbId.empty();
}

void Resource::swap(Resource& r) {
	mDbId.swap(r.mDbId);
	std::swap(mType, r.mType);
	std::swap(mDuration, r.mDuration);
	std::swap(mWidth, r.mWidth);
	std::swap(mHeight, r.mHeight);
	mFileName.swap(r.mFileName);
	mPath.swap(r.mPath);
	std::swap(mThumbnailId, r.mThumbnailId);
	mLocalFilePath.swap(r.mLocalFilePath);
}

bool Resource::isLocal() const
{
	// parameters mFileName and mPath correspond to their fields in the resources table
	return !mLocalFilePath.empty() && mFileName.empty() && mPath.empty();
}

bool Resource::query(const Resource::Id& id) {
	const std::string&          dbPath = id.getDatabasePath();
	if (dbPath.empty()) return false;

	std::stringstream           buf;
	buf.str("");
	buf << "SELECT resourcestype,resourcesduration,resourceswidth,resourcesheight,resourcesfilename,resourcespath,resourcesthumbid FROM Resources WHERE resourcesid = " << id.mValue;
	query::Result               r;
	if (!query::Client::query(dbPath, buf.str(), r) || r.rowsAreEmpty()) {
		return false;
	}

	clear();

	query::Result::RowIterator  it(r);
	if (!it.hasValue()) return false;

	setDbId(id);
	setTypeFromString(it.getString(0));
	mDuration = it.getFloat(1);
	mWidth = it.getFloat(2);
	mHeight = it.getFloat(3);
	mFileName = it.getString(4);
	mPath = it.getString(5);
	mThumbnailId = it.getInt(6);

	return true;
}

bool Resource::query(const Resource::Id& id, Resource* outThumb) {
	const bool		ans = query(id);
	if (ans && mThumbnailId > 0 && outThumb) {
		ds::Resource::Id	tid(mDbId);
		tid.mValue = mThumbnailId;
		outThumb->query(tid);
	}
	return ans;
}

void Resource::setTypeFromString(const std::string& typeChar) {
	mType = makeTypeFromString(typeChar);
}


const int Resource::makeTypeFromString(const std::string& typeChar){
	if(FONT_TYPE_SZ == typeChar) return FONT_TYPE;
	else if(IMAGE_TYPE_SZ == typeChar) return IMAGE_TYPE;
	else if(IMAGE_SEQUENCE_TYPE_SZ == typeChar) return IMAGE_SEQUENCE_TYPE;
	else if(PDF_TYPE_SZ == typeChar) return PDF_TYPE;
	else if(VIDEO_TYPE_SZ == typeChar) return VIDEO_TYPE;
	else if(VIDEO_STREAM_TYPE_SZ == typeChar) return VIDEO_STREAM_TYPE;
	else if(WEB_TYPE_SZ == typeChar) return WEB_TYPE;
	else if(AUDIO_TYPE_SZ == typeChar) return VIDEO_TYPE;
	else return ERROR_TYPE;
}

const int Resource::parseTypeFromFilename(const std::string& newMedia){

	// creating a Poco::File from an empty string and performing
	// any checks throws a runtime exception
	if(newMedia.empty()){
		return ds::Resource::ERROR_TYPE;
	}

	if(newMedia.find("http") == 0){
		return ds::Resource::WEB_TYPE;
	}

	if(newMedia.find("udp") == 0
		|| newMedia.find("rtsp") == 0){
		return ds::Resource::VIDEO_STREAM_TYPE;
	}

	if(!ds::safeFileExistsCheck(newMedia, false)){
		return ds::Resource::ERROR_TYPE;
	}

	Poco::File filey = Poco::File(newMedia);

	std::string extensionay = Poco::Path(filey.path()).getExtension();
	std::transform(extensionay.begin(), extensionay.end(), extensionay.begin(), ::tolower);
	if(extensionay.find("gif") != std::string::npos){
		return ds::Resource::WEB_TYPE;

	} else if(extensionay.find("pdf") != std::string::npos){
		return ds::Resource::PDF_TYPE;

	} else if(extensionay.find("png") != std::string::npos
			  || extensionay.find("jpg") != std::string::npos
			  || extensionay.find("jpeg") != std::string::npos
			  ){
		return ds::Resource::IMAGE_TYPE;

	} else if(extensionay.find("ttf") != std::string::npos
				|| extensionay.find("otf") != std::string::npos){
		return ds::Resource::FONT_TYPE;
	} else if(extensionay.find("mov") != std::string::npos
			  || extensionay.find("mp4") != std::string::npos
			  || extensionay.find("mp3") != std::string::npos
			  || extensionay.find("wav") != std::string::npos
			  || extensionay.find("avi") != std::string::npos
			  || extensionay.find("wmv") != std::string::npos
			  || extensionay.find("flv") != std::string::npos
			  || extensionay.find("m4v") != std::string::npos
			  || extensionay.find("mpg") != std::string::npos
			  || extensionay.find("mkv") != std::string::npos
			  || extensionay.find("3gp") != std::string::npos
			  || extensionay.find("webm") != std::string::npos
			  || extensionay.find("asf") != std::string::npos
			  || extensionay.find("dat") != std::string::npos
			  || extensionay.find("divx") != std::string::npos
			  || extensionay.find("dv") != std::string::npos
			  || extensionay.find("f4v") != std::string::npos
			  || extensionay.find("m2ts") != std::string::npos
			  || extensionay.find("mod") != std::string::npos
			  || extensionay.find("mpe") != std::string::npos
			  || extensionay.find("ogv") != std::string::npos
			  || extensionay.find("mpeg") != std::string::npos
			  || extensionay.find("mts") != std::string::npos
			  || extensionay.find("nsv") != std::string::npos
			  || extensionay.find("ogm") != std::string::npos
			  || extensionay.find("qt") != std::string::npos
			  || extensionay.find("tod") != std::string::npos
			  || extensionay.find("ts") != std::string::npos
			  || extensionay.find("vob") != std::string::npos
			  ){
		return ds::Resource::VIDEO_TYPE;
	} else {
		return ds::Resource::ERROR_TYPE;
	}
}

} // namespace ds

/**
 * ds::resource_id stream printing
 */
std::ostream& operator<<(std::ostream& os, const ds::Resource::Id& o)
{
	if (o.mType == o.CMS_TYPE)		os << "cms:";
	else if (o.mType == o.APP_TYPE)	os << "app:";
	else							os << "error:";
	return os << o.mValue;
}

std::wostream& operator<<(std::wostream& os, const ds::Resource::Id& o)
{
	if (o.mType == o.CMS_TYPE)		os << L"cms:";
	else if (o.mType == o.APP_TYPE)	os << L"app:";
	else							os << L"error:";
	return os << o.mValue;
}
