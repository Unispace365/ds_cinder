#pragma once
#ifndef DS_DATA_RESOURCE_H_
#define DS_DATA_RESOURCE_H_

#include <functional>
#include <string>

namespace ds {
class DataBuffer;
class ResourceList;

/**
 * \class ds::Resource
 * \brief Encapsulate a row in the Resources table.
 */
class Resource
{
public:
	/**
	 * \struct ds::Resource::id
	 * \brief A single resource ID, composed of a type and value.
	 * Resources can exist in one of several databases, hence the type.
	 */
	struct Id {
		// The traditional type for remotely-generated user content,
		// but also the only type supported until this structure was added.
		//  All legacy projects will have this type and only this type.
		static const char	CMS_TYPE = 0;
		// Local application resources like UI components.
		static const char	APP_TYPE = 1;
		// Custom database types can be this value or less.
		static const char	CUSTOM_TYPE = -32;

		// All framework-defined types will be positive.  Negative values
		// are reserved for applications to create fake resource_ids.
		char				mType;
		int					mValue;

		Id();
		Id(const int value);
		Id(const char type, const int value);

		bool				operator==(const Id&) const;
		bool				operator!=(const Id&) const;
		bool				operator>(const Id&) const;
		bool				operator<(const Id&) const;
		// Comparison with a raw value
		bool				operator>(const int value) const;
		bool				operator>=(const int value) const;
		bool				operator<(const int value) const;
		bool				operator<=(const int value) const;

		bool				empty() const;
		void				clear();
		void				swap(Id&);

		// Assumes the string is in my string output format (type:value)
		bool				tryParse(const std::string&);

		// Get the path to the resources for this type, or directly to the
		// database (which will always be in the resource path).
		const std::string&	getResourcePath() const;
		const std::string&	getDatabasePath() const;
		// These are used when transporting a resource across a network.
		// They can be resolved to correct paths with ds::Environment::expand()
		const std::string&	getPortableResourcePath() const;

		// Utility to report and log an error if I'm missing path information
		bool				verifyPaths() const;

		void			    writeTo(DataBuffer&) const;
		bool			    readFrom(DataBuffer&);

		// The engines are required to set paths to the various resource database before
		// anyone does anything.  This assumes the traditional CMS path -- a resource
		// location and a database file inside it.  From this info, the legacy and CMS
		// (which are currently the same thing) paths are set, and the files are probed
		// to set any additional paths.
		// root and have the newer db structure figured automatically.
		// OK!  resource and db are now considered legacy values.  Project path will
		// have a segment of the path to the resources -- it won't have the start,
		// which is standardized to My Documents\downstream\resources, and it won't have
		// the end, which is now a common structure.  Currently this only applies to
		// app-local resources, the traditional CMS resources have the traditional CMS setup.
		static void			setupPaths(	const std::string& resource, const std::string& db,
										const std::string& projectPath);
		// The client app can set a function responsible for returning the paths to any
		// custom database types.
		// NOTE:  For efficiency, always return a valid string ref, even if it's on an empty string.
		// Never return a string newly constructed in the function.
		static void         setupCustomPaths(	const std::function<const std::string&(const Resource::Id&)>& resourcePath,
												const std::function<const std::string&(const Resource::Id&)>& dbPath);
	};

public:
	static const int		ERROR_TYPE          = 0;
	static const int		FONT_TYPE           = 1;
	static const int		IMAGE_TYPE          = 2;
	static const int		IMAGE_SEQUENCE_TYPE = 3;
	static const int		PDF_TYPE            = 4;
	static const int		VIDEO_TYPE          = 5;
	static const int		WEB_TYPE            = 6;

public:
	// Mainly for debugging
	static Resource			fromImage(const std::string& full_path);
	static Resource			fromQuery(const Resource::Id&);

	Resource();
	Resource(const Resource::Id& dbId, const int type);
	// This should only be used for debugging, as a way to bridge
	// the hardcoded file paths before things move to being pulled from the CMS.
	Resource(const std::string& fullPath); // type is auto-detected from extension
	Resource(const std::string& fullPath, const int type);

	// In case you have this queried/constructed already
	Resource(const Resource::Id dbid, const int type, const double duration, 
			 const float width, const float height, const std::string filename, 
			 const std::string path, const int thumbnailId, const std::string debugFileName);

	bool					operator==(const Resource&) const;
	bool					operator!=(const Resource&) const;

	const Resource::Id&		getDbId() const			    { return mDbId; }
	int						getType() const			    { return mType; }
	const std::wstring&		getTypeName() const;
	double					getDuration() const     { return mDuration; }
	float					getWidth() const        { return mWidth; }
	float					getHeight() const       { return mHeight; }
	int						getThumbnailId() const  { return mThumbnailId; }
	// Answer the full path to my file
	std::string				getAbsoluteFilePath() const;
	// Answer an abstract file path that can be resolved to an absolute
	// one via ds::Environment::expand().
	std::string				getPortableFilePath() const;

	/** If you want to simply store a path to a thumbnail
	*	This is NOT filled out by default in the query() methods, you need to supply this yourself
	**/
	std::string				getThumbnailFilePath() const { return mThumbnailFilePath; }
	void					setThumbnailFilePath(const std::string& thumbPath) { mThumbnailFilePath = thumbPath; }

	void					clear();
	bool					empty() const;
	void					swap(Resource&);

	void					setDbId(const Resource::Id&);
	void					setType(const int);
	void					setTypeFromString(const std::string& typeChar);

	static const int		parseTypeFromFilename(const std::string& fileName);

	// Warning: Expensive operation (database lookup).  Use with care.
	bool					existsInDb() const;
	// Answers true if ds::Resource was constructed from a local
	// file instead of an actual element in db. via fromImage method for example.
	bool					isLocal() const;
	// Query the DB for my contents. Obviously, this is also an expensive operation.
	bool					query(const Resource::Id&);
	// The argument is the full thumbnail, if you want it.
	bool					query(const Resource::Id&, Resource* outThumb);

private:
	friend class ResourceList;

	Resource::Id			mDbId;
	int						mType;
	double					mDuration;
	float					mWidth,
							mHeight;
	std::string				mFileName;
	std::string				mPath;
	// Sorta hacked in for Kurt's model
	int						mThumbnailId;
	// Only should be used for debugging
	std::string				mDebugFileName;

	std::string				mThumbnailFilePath;

};

} // namespace ds

// Make the resource ID available to standard stream operators
std::ostream&             operator<<(std::ostream&, const ds::Resource::Id&);
std::wostream&            operator<<(std::wostream&, const ds::Resource::Id&);

/*\cond Have doxygen ignore this, since it's an internal function that pops the std namespace on the main list
		 Make the resource ID available for hashing functions
*/
namespace std {
template<>
struct hash<ds::Resource::Id> : public unary_function < ds::Resource::Id, size_t > {
	size_t operator()(const ds::Resource::Id& id) const {
		return id.mType + (id.mValue << 8);
	}
};
/* \endcond */
}

#endif // DS_DATA_RESOURCE_H_