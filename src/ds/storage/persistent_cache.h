#pragma once
#ifndef DS_STORAGE_PERSISTENTCACHE_H_
#define DS_STORAGE_PERSISTENTCACHE_H_

#include <string>
#include <vector>
#include <cinder/Thread.h>

namespace ds {

/**
 * \class gpw::PersistentCache
 * \brief Abstract persistent storage. Define a data format, then add and query.
 * I am thread safe (meaning I block on all calls).
 */
class PersistentCache {
public:
	class FieldFormat {
	public:
		enum						Type { kFloat, kInt, kString };
		FieldFormat() : mType(kString)	{ }
		FieldFormat(const std::string& name, const Type& t) : mName(name), mType(t)	{ }
		std::string					mName;
		Type						mType;
	};
	class FieldList {
	public:
		FieldList();
		FieldList&					addFloat(const std::string& name);
		FieldList&					addInt(const std::string& name);
		FieldList&					addString(const std::string& name);
		std::vector<FieldFormat>	mFields;
	};

public:
	class Field;
	class Row;

	// Location will be relative to user/documents/downstream/cache. Location
	// should be a folder -- the file will be named and generated.
	// Version is currently unused, but maintain it for the future.
	// For convenience you can use field list like this: PersistentCache::FieldList().addString("query")
	PersistentCache(const std::string& location, const int version, const FieldList&);

	Row								fetchOne(const std::string& field_name, const std::string& value) const;
	// If the row has an ID, this is an update operation, otherwise this is a create.
	void							setValues(const Row&);

private:
	void							verifyDatabase(const int version, const FieldList& list);
	void							loadDatabase(const FieldList& list);

	PersistentCache();
	PersistentCache(const PersistentCache&);

	const std::string				mFilename;

public:
	class Field {
	public:
		Field();
		Field(const double, const int64_t, const std::string&);

		double						mFloat;
		int64_t						mInt;
		std::string					mString;
	};
	class Row {
	public:
		Row();

		bool						empty() const;

		double						getFloat(const size_t) const;
		int64_t						getInt(const size_t) const;
		const std::string&			getString(const size_t) const;

		// For building
		Row&						addFloat(const double);
		Row&						addInt(const int64_t);
		Row&						addString(const std::string&);

		int							mId;
		std::vector<Field>			mFields;
	};

private:
	const FieldList					mFieldFormats;
	mutable std::mutex				mMutex;
	std::vector<Row>				mRows;
};

} // namespace ds

#endif
