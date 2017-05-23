#include "stdafx.h"

#include "persistent_cache.h"

#include <sstream>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <ds/query/query_client.h>
#include <ds/query/query_result.h>

namespace ds {

namespace {
const std::string	EMPTY_SZ;

std::string			make_filename(const std::string& location) {
	Poco::Path		p(Poco::Path::home());
	p.append("documents").append("downstream").append("cache").append(location);
	p = Poco::Path(Poco::Path::expand(p.toString()));
	Poco::File		f(p.toString());
	if (!f.exists()) f.createDirectories();
	p.append("db.sqlite");
	return p.toString();
}

} // anonymous namespace

/**
 * \class ds::PersistentCache
 */
PersistentCache::PersistentCache(const std::string& location, const int version, const FieldList& list)
		: mFilename(make_filename(location))
		, mFieldFormats(list) {
	verifyDatabase(version, list);
	loadDatabase(list);
}

PersistentCache::Row PersistentCache::fetchOne(const std::string& field_name, const std::string& value) const {
	size_t						idx = mFieldFormats.mFields.size()+100;
	for (size_t k=0; k<mFieldFormats.mFields.size(); ++k) {
		const FieldFormat&		fmt(mFieldFormats.mFields[k]);
		if (fmt.mName == field_name) {
			idx = k;
			break;
		}
	}
	if (idx >= mFieldFormats.mFields.size()) return Row();

	std::unique_lock<std::mutex>		lock(mMutex);
	for (auto it=mRows.begin(), end=mRows.end(); it!=end; ++it) {
		const Row&				r(*it);
		if (r.mFields[idx].mString == value) return r;
	}
	return Row();
}

void PersistentCache::setValues(const Row& row) {
	ds::query::Result					ans;

	// UPDATE
	if (row.mId > 0) {
		std::stringstream				buf;
		buf << "UPDATE cache SET ";
		for (size_t k=0; k<mFieldFormats.mFields.size(); ++k) {
			const FieldFormat::Type		type(mFieldFormats.mFields[k].mType);
			if (k != 0) buf << ", ";
			buf << mFieldFormats.mFields[k].mName << "=";
			if (type == FieldFormat::kFloat) {
				buf << row.getFloat(k);
			} else if (type == FieldFormat::kInt) {
				buf << row.getInt(k);
			} else if (type == FieldFormat::kString) {
				buf << "'" << row.getString(k) << "'";
			}
		}
		buf << " WHERE id=" << row.mId;
		ds::query::Client::queryWrite(mFilename, buf.str(), ans);

	// CREATE
	} else {
		std::stringstream		buf_1, buf_2;
		buf_1 << "INSERT INTO cache (";
		for (size_t k=0; k<mFieldFormats.mFields.size(); ++k) {
			const FieldFormat::Type		type(mFieldFormats.mFields[k].mType);
			if (k != 0) {
				buf_1 << ", ";
				buf_2 << ", ";
			}
			buf_1 << mFieldFormats.mFields[k].mName;
			if (type == FieldFormat::kFloat) {
				buf_2 << row.getFloat(k);
			} else if (type == FieldFormat::kInt) {
				buf_2 << row.getInt(k);
			} else if (type == FieldFormat::kString) {
				buf_2 << "'" << row.getString(k) << "'";
			}
		}
		buf_1 << ") values (" << buf_2.str() << ")";
		ds::query::Client::queryWrite(mFilename, buf_1.str(), ans);
	}

	// sync
	std::unique_lock<std::mutex>		lock(mMutex);
	loadDatabase(mFieldFormats);
}

void PersistentCache::verifyDatabase(const int version, const FieldList& list) {
	if (mFilename.empty()) return;

	Poco::File			f(mFilename);
	// XXX Check for version number
	if (f.exists()) return;

	f.createFile();

	std::stringstream	buf;
	buf << "CREATE TABLE cache(id INTEGER PRIMARY KEY AUTOINCREMENT";
	for (auto it=list.mFields.begin(), end=list.mFields.end(); it!=end; ++it) {
		if (it->mName.empty()) throw std::runtime_error("PersistentCache::verifyDatabase() empty field name");
		if (it->mType == it->kFloat) {
			buf << ", " << it->mName << " DOUBLE NOT NULL DEFAULT '0'";
		} else if (it->mType == it->kInt) {
			buf << ", " << it->mName << " INT NOT NULL DEFAULT '0'";
		} else if (it->mType == it->kString) {
			buf << ", " << it->mName << " TEXT NOT NULL DEFAULT ''";
		}
	}
	buf << ");";
	ds::query::Result	r;
	ds::query::Client::queryWrite(mFilename, buf.str(), r);
}

void PersistentCache::loadDatabase(const FieldList& list) {
	mRows.clear();

	std::stringstream				buf;
	buf << "SELECT id";
	for (auto it=list.mFields.begin(), end=list.mFields.end(); it!=end; ++it) {
		if (it->mName.empty()) throw std::runtime_error("PersistentCache::loadDatabase() empty field name");
		buf << "," << it->mName;
	}
	buf << " FROM cache";

	ds::query::Result				ans;
	ds::query::Client::query(mFilename, buf.str(), ans);
	ds::query::Result::RowIterator	it(ans);
	while (it.hasValue()) {
		mRows.push_back(Row());
		Row&						row(mRows.back());
		row.mId = it.getInt(0);
		for (int k=0; k<(int)list.mFields.size(); ++k) {
			const FieldFormat&		fmt(list.mFields[k]);
			if (fmt.mType == fmt.kFloat) {
				row.mFields.push_back(Field(it.getFloat(k+1), 0, ""));
			} else if (fmt.mType == fmt.kInt) {
				row.mFields.push_back(Field(0.0, it.getInt(k+1), ""));
			} else if (fmt.mType == fmt.kString) {
				row.mFields.push_back(Field(0.0, 0, it.getString(k+1)));
			}
		}
		++it;
	}
}

/**
 * \class ds::PersistentCache::FieldList
 */
PersistentCache::FieldList::FieldList() {
}

PersistentCache::FieldList& PersistentCache::FieldList::addFloat(const std::string& name) {
	mFields.push_back(FieldFormat(name, FieldFormat::kFloat));
	return *this;
}

PersistentCache::FieldList& PersistentCache::FieldList::addInt(const std::string& name) {
	mFields.push_back(FieldFormat(name, FieldFormat::kInt));
	return *this;
}

PersistentCache::FieldList& PersistentCache::FieldList::addString(const std::string& name) {
	mFields.push_back(FieldFormat(name, FieldFormat::kString));
	return *this;
}

/**
 * \class ds::PersistentCache::Field
 */
PersistentCache::Field::Field() {
}

PersistentCache::Field::Field(const double v1, const int64_t v2, const std::string& v3)
		: mFloat(v1)
		, mInt(v2)
		, mString(v3) {
}

/**
 * \class ds::PersistentCache::Row
 */
PersistentCache::Row::Row()
		: mId(0) {
}

bool PersistentCache::Row::empty() const {
	return mId < 1;
}

double PersistentCache::Row::getFloat(const size_t idx) const {
	if (idx >= mFields.size()) return 0.0;
	return mFields[idx].mFloat;
}

int64_t PersistentCache::Row::getInt(const size_t idx) const {
	if (idx >= mFields.size()) return 0;
	return mFields[idx].mInt;
}

const std::string& PersistentCache::Row::getString(const size_t idx) const {
	if (idx >= mFields.size()) return EMPTY_SZ;
	return mFields[idx].mString;
}

PersistentCache::Row& PersistentCache::Row::addFloat(const double v) {
	mFields.push_back(Field(v, 0, ""));
	return *this;
}

PersistentCache::Row& PersistentCache::Row::addInt(const int64_t v) {
	mFields.push_back(Field(0.0, v, 0));
	return *this;
}

PersistentCache::Row& PersistentCache::Row::addString(const std::string& v) {
	mFields.push_back(Field(0.0, 0, v));
	return *this;
}

} // namespace ds
