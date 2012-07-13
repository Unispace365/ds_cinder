#include "ds/data/resource.h"

#include <iostream>
#include <Poco/Path.h>
#include "ds/app/environment.h"
#include "ds/debug/debug_defines.h"

namespace {
const std::string		  FONT_TYPE_SZ("f");
const std::string		  IMAGE_TYPE_SZ("i");
const std::string		  IMAGE_SEQUENCE_TYPE_SZ("s");
const std::string		  PDF_TYPE_SZ("p");
const std::string		  VIDEO_TYPE_SZ("v");
const std::string		  WEB_TYPE_SZ("w");

const std::string		  EMPTY_SZ("");

const std::wstring		FONT_NAME_SZ(L"font");
const std::wstring		IMAGE_NAME_SZ(L"image");
const std::wstring		IMAGE_SEQUENCE_NAME_SZ(L"image sequence");
const std::wstring		PDF_NAME_SZ(L"pdf");
const std::wstring		VIDEO_NAME_SZ(L"video");
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

/**
 * ds::Resource::id database path
 */
namespace {
std::string				  CMS_RESOURCE_PATH("");
std::string				  CMS_DB_PATH("");
std::string				  APP_RESOURCE_PATH("");
std::string				  APP_DB_PATH("");
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

void Resource::Id::setupPaths(const std::string& resource, const std::string& db,
                              const std::string& projectPath)
{
	CMS_RESOURCE_PATH = resource;
	CMS_DB_PATH = db;

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
Resource::Resource()
	: mType(ERROR_TYPE)
  , mDuration(0)
  , mWidth(0)
  , mHeight(0)
{
}

Resource::Resource(const Resource::Id& dbId, const int type)
	: mDbId(dbId)
	, mType(type)
  , mDuration(0)
  , mWidth(0)
  , mHeight(0)
{
}

const std::wstring& Resource::getTypeName() const
{
	if (mType == FONT_TYPE) return FONT_NAME_SZ;
	else if (mType == IMAGE_TYPE) return IMAGE_NAME_SZ;
	else if (mType == IMAGE_SEQUENCE_TYPE) return IMAGE_SEQUENCE_NAME_SZ;
	else if (mType == PDF_TYPE) return PDF_NAME_SZ;
	else if (mType == VIDEO_TYPE) return VIDEO_NAME_SZ;
	else if (mType == WEB_TYPE) return WEB_NAME_SZ;
	return ERROR_NAME_SZ;
}

std::string Resource::getAbsoluteFilePath() const
{
  if (mFileName.empty()) return EMPTY_SZ;
  Poco::Path        p(mDbId.getResourcePath());
  if (p.depth() < 1) return EMPTY_SZ;
  p.append(mPath).append(mFileName);
  return p.toString();
}

void Resource::setDbId(const Resource::Id& id)
{
	mDbId = id;
}

void Resource::setType(const int type)
{
	mType = type;
}

bool Resource::existsInDb() const
{
return false;
#if 0
	static const std::string		SELECT("SELECT resourcesid FROM Resources WHERE resourcesid = ");
	std::stringstream				buf;
	QueryResult						r;
	buf << SELECT << mDbId.mValue;
	if (!QueryClient::query(mDbId.getDatabasePath(), buf.str(), r) || r.rowsAreEmpty()) return false;
	return true;
#endif
}

void Resource::setTypeFromString(const std::string& typeChar)
{
	if (FONT_TYPE_SZ == typeChar) mType = FONT_TYPE;
	else if (IMAGE_TYPE_SZ == typeChar) mType = IMAGE_TYPE;
	else if (IMAGE_SEQUENCE_TYPE_SZ == typeChar) mType = IMAGE_SEQUENCE_TYPE;
	else if (PDF_TYPE_SZ == typeChar) mType = PDF_TYPE;
	else if (VIDEO_TYPE_SZ == typeChar) mType = VIDEO_TYPE;
	else if (WEB_TYPE_SZ == typeChar) mType = WEB_TYPE;
	else mType = ERROR_TYPE;
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
