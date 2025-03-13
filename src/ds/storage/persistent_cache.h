#pragma once
#ifndef DS_STORAGE_PERSISTENTCACHE_H_
#define DS_STORAGE_PERSISTENTCACHE_H_

#include <string>
#include <vector>

namespace ds {

/**
 * \class PersistentCache
 * \brief Abstract persistent storage. Define a data format, then add and query.
 * I am thread safe (meaning I block on all calls).
 */
class PersistentCache {
  public:
	class FieldFormat {
	  public:
		enum Type { kFloat, kInt, kString };
		FieldFormat()
		  : mType(kString) {}
		FieldFormat(const std::string& name, const Type& t)
		  : mName(name)
		  , mType(t) {}
		std::string mName;
		Type		mType;
	};
	class FieldList {
	  public:
		FieldList() = default;
		FieldList&				 addFloat(const std::string& name);
		FieldList&				 addInt(const std::string& name);
		FieldList&				 addString(const std::string& name);
		std::vector<FieldFormat> mFields;
	};

  public:
	class Field;
	class Row;

	PersistentCache()								   = delete;
	PersistentCache(const PersistentCache&)			   = delete;
	PersistentCache(PersistentCache&&)				   = delete;
	PersistentCache& operator=(const PersistentCache&) = delete;
	PersistentCache& operator=(PersistentCache&&)	   = delete;

	/// Location will be relative to user/documents/downstream/cache. Location
	/// should be a folder -- the file will be named and generated.
	/// Version is currently unused, but maintain it for the future.
	/// For convenience you can use field list like this: PersistentCache::FieldList().addString("query")
	PersistentCache(const std::string& location, int version, const FieldList&);

	Row fetchOne(const std::string& field_name, const std::string& value) const;
	/// If the row has an ID, this is an update operation, otherwise this is a create.
	void setValues(const Row&);

  private:
	void verifyDatabase(int version, const FieldList& list) const;
	void loadDatabase(const FieldList& list);

	const std::string mFilename;

  public:
	class Field {
	  public:
		Field() = default;
		Field(double, int64_t, const std::string&);

		double		mFloat;
		int64_t		mInt;
		std::string mString;
	};
	class Row {
	  public:
		Row();

		bool empty() const;

		double			   getFloat(size_t) const;
		int64_t			   getInt(size_t) const;
		const std::string& getString(size_t) const;

		/// For building
		Row& addFloat(double);
		Row& addInt(int64_t);
		Row& addString(const std::string&);

		int				   mId;
		std::vector<Field> mFields;
	};

  private:
	const FieldList	   mFieldFormats;
	mutable std::mutex mMutex;
	std::vector<Row>   mRows;
};

} // namespace ds

#endif
