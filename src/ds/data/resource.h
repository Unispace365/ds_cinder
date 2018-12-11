#pragma once
#ifndef DS_DATA_RESOURCE_H_
#define DS_DATA_RESOURCE_H_

#include <functional>
#include <string>
#include <vector>

namespace ds {
class DataBuffer;
class ResourceList;

/**
 * \class Resource
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
		/// The traditional type for remotely-generated user content,
		/// but also the only type supported until this structure was added.
		///  All legacy projects will have this type and only this type.
		static const char	CMS_TYPE = 0;
		/// Local application resources like UI components.
		static const char	APP_TYPE = 1;
		/// Custom database types can be this value or less.
		static const char	CUSTOM_TYPE = -32;

		/// All framework-defined types will be positive.  Negative values
		/// are reserved for applications to create fake resource_ids.
		char				mType;
		int					mValue;

		Id();
		Id(const int value);
		Id(const char type, const int value);

		bool				operator==(const Id&) const;
		bool				operator!=(const Id&) const;
		bool				operator>(const Id&) const;
		bool				operator<(const Id&) const;
		/// Comparison with a raw value
		bool				operator>(const int value) const;
		bool				operator>=(const int value) const;
		bool				operator<(const int value) const;
		bool				operator<=(const int value) const;

		bool				empty() const;
		void				clear();
		void				swap(Id&);

		/// Assumes the string is in my string output format (type:value)
		bool				tryParse(const std::string&);

		/// Get the path to the resources for this type, or directly to the
		/// database (which will always be in the resource path).
		const std::string&	getResourcePath() const;
		const std::string&	getDatabasePath() const;
		/// These are used when transporting a resource across a network.
		/// They can be resolved to correct paths with ds::Environment::expand()
		const std::string&	getPortableResourcePath() const;

		/// Utility to report and log an error if I'm missing path information
		bool				verifyPaths() const;

		void			    writeTo(DataBuffer&) const;
		bool			    readFrom(DataBuffer&);

		/// The engines are required to set paths to the various resource database before
		/// anyone does anything.  This assumes the traditional CMS path -- a resource
		/// location and a database file inside it.  From this info, the legacy and CMS
		/// (which are currently the same thing) paths are set, and the files are probed
		/// to set any additional paths.
		/// root and have the newer db structure figured automatically.
		/// OK!  resource and db are now considered legacy values.  Project path will
		/// have a segment of the path to the resources -- it won't have the start,
		/// which is standardized to `My Documents\downstream\resources`, and it won't have
		/// the end, which is now a common structure.  Currently this only applies to
		/// app-local resources, the traditional CMS resources have the traditional CMS setup.
		static void			setupPaths(	const std::string& resource, const std::string& db,
										const std::string& projectPath);
		/// The client app can set a function responsible for returning the paths to any
		/// custom database types.
		/// NOTE:  For efficiency, always return a valid string ref, even if it's on an empty string.
		/// Never return a string newly constructed in the function.
		static void         setupCustomPaths(	const std::function<const std::string&(const Resource::Id&)>& resourcePath,
												const std::function<const std::string&(const Resource::Id&)>& dbPath);
	};

public:
	static const int		ERROR_TYPE				= 0;
	static const int		FONT_TYPE				= 1;
	static const int		IMAGE_TYPE				= 2;
	static const int		IMAGE_SEQUENCE_TYPE		= 3;
	static const int		PDF_TYPE				= 4;
	static const int		VIDEO_TYPE				= 5;
	static const int		WEB_TYPE				= 6;
	static const int		VIDEO_STREAM_TYPE		= 7;
	static const int		ZIP_TYPE				= 8;
	static const int		VIDEO_PANORAMIC_TYPE	= 9;

public:
	/// Mainly for debugging
	static Resource			fromImage(const std::string& full_path);
	static Resource			fromQuery(const Resource::Id&);

	Resource();
	Resource(const Resource::Id& dbId, const int type);

	/// Sets the absolute filepath, type is auto-detected, no other parameters are filled out
	Resource(const std::string& localFullPath);
	/// Sets the absolute filepath, no other parameters are filled out
	Resource(const std::string& localFullPath, const int type);
	/// Sets the absolute filepath, type is auto-detected. This is intended for streams
	Resource(const std::string& localFullPath, const float width, const float height);

	/// In case you have this queried/constructed already
	Resource(const Resource::Id dbid, const int type, const double duration, 
			 const float width, const float height, const std::string filename, 
			 const std::string path, const int thumbnailId, const std::string fullFilePath);

	bool					operator==(const Resource&) const;
	bool					operator!=(const Resource&) const;

	const Resource::Id&		getDbId() const						{ return mDbId; }
	void					setDbId(const Resource::Id& dbId)	{ mDbId = dbId; }

	const std::wstring&		getTypeName() const;
	const std::string&		getTypeChar() const;
	int						getType() const						{ return mType; }
	void					setType(const int newType)			{ mType = newType; }

	double					getDuration() const					{ return mDuration; }
	void					setDuration(const float newDur)		{ mDuration = newDur; }

	float					getWidth() const					{ return mWidth; }
	void					setWidth(const float newWidth)		{ mWidth = newWidth; }

	float					getHeight() const					{ return mHeight; }
	void					setHeight(const float newHeight)	{ mHeight = newHeight; }

	int						getThumbnailId() const				{ return mThumbnailId; }
	void					setThumbnailId(const int thub)		{ mThumbnailId = thub; }

	/// If this resource has a parent (like pages of a PDF), get the ID for the parent
	int						getParentId() const					{ return mParentId; }
	void					setParentId(const int parentId)		{ mParentId = parentId; }

	/// The sort order of this resource in it's parent
	int						getParentIndex() const				{ return mParentIndex; }
	void					setParentIndex(const int parentIndx){ mParentIndex = parentIndx; }

	std::vector<Resource>&	getChildrenResources()				{ return mChildrenResources; }
	void					setChildrenResources(const std::vector<Resource>& newChildren){ mChildrenResources = newChildren; }

	/// If you want to simply store a path to a thumbnail
	///	This is NOT filled out by default in the query() methods, you need to supply this yourself	
	std::string				getThumbnailFilePath() const { return mThumbnailFilePath; }
	void					setThumbnailFilePath(const std::string& thumbPath) { mThumbnailFilePath = thumbPath; }

	/// Answer the full path to my file
	std::string				getAbsoluteFilePath() const;

	/// Local file path is the path to a file, generally not tracked by a database. This will be used instead of resource ID, and FileName and Path won't be used.
	void					setLocalFilePath(const std::string& localPath, const bool normalizeThePath = true);

	/// Answer an abstract file path that can be resolved to an absolute one via ds::Environment::expand().
	std::string				getPortableFilePath() const;

	std::string				getFileName() const { return mFileName; }
	void					setFileName(const std::string& fileName){ mFileName = fileName; }

	/// Clears the currently set info
	void					clear();

	/// If anything has been set
	bool					empty() const;
	void					swap(Resource&);

	/// Expects a single-character type (v, i, p, w, f, s)
	void					setTypeFromString(const std::string& typeChar);
	/// Return the int value for the string type
	static const int		makeTypeFromString(const std::string& typeChar);

	/// Returns the type parsed from the filename, primarily using the file extension.
	/// Creates an error type if it's a file type (not web type) and the file doesn't exist
	/// Use the full file path or web URL, not a single character like above
	static const int		parseTypeFromFilename(const std::string& fileName);

	/// Answers true if ds::Resource was constructed from a local
	/// file instead of an actual element in db. via fromImage method for example.
	bool					isLocal() const;

	/// Query the database set as the resources database for my contents. Obviously, this is also an expensive operation.
	bool					query(const Resource::Id&);

	/// The argument is the full thumbnail, if you want it.
	bool					query(const Resource::Id&, Resource* outThumb);

private:
	friend class ResourceList;

	Resource::Id			mDbId;

	/// See the public types above
	int						mType;
	double					mDuration;
	float					mWidth,
							mHeight;

	/// filename and path are applied when querying from a database
	std::string				mFileName;
	std::string				mPath;

	/// Overrides FileName and Path
	std::string				mLocalFilePath;

	/// thumbnail id used when querying from a database
	int						mThumbnailId;
	/// can be set manually as a convenience
	std::string				mThumbnailFilePath;

	/// Some resources are representations of another resource (like pages of a pdf), this lets you link the two
	int						mParentId;
	int						mParentIndex;
	std::vector<Resource>	mChildrenResources;


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
