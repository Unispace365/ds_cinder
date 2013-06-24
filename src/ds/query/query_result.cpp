#include "ds/query/query_result.h"

#include <iostream>
#include <stdarg.h>
#include <math.h>

using namespace std;

namespace {
const string				RESULT_EMPTY_STR("");
const wstring				RESULT_EMPTY_WSTR(L"");
}

#ifndef WIN32
	#define _ASSERT assert
#endif

namespace ds {

namespace query {

const int					QUERY_NO_TYPE = 0;
const int					QUERY_STRING = 1;
const int					QUERY_NUMERIC = 2;
const int					QUERY_NULL = 3;

/* QUERY-RESULT::STRING-FACTORY
 * Localize memory management of string instances.
 ******************************************************************/
class Result::StringFactory : public ManagedFactory<string*>
{
public:
	StringFactory() { }

	virtual string*			newObject() const {
		string*				ans = new string();
		if (ans == NULL) throw std::exception();
		return ans;
	}

	virtual string*			nullObject() const { return NULL; }
	virtual void			deleteObject(string* p) const { delete p; }
	virtual void			initializeObject(string* p) const { if (p) p->clear(); }
	virtual void			setObject(string* dst, string* src) const { assert(false); }
};
static Result::StringFactory	STRING_FACTORY;

/* QUERY-RESULT::ROW-FACTORY
 * Localize memory management of Row instances.
 ******************************************************************/
class Result::RowFactory : public ManagedFactory<Result::Row*>
{
public:
	RowFactory() { }

	virtual Row*			newObject() const {
		Row*				ans = new Row();
		if (ans == NULL) throw std::exception();
		return ans;
	}

	virtual Row*			nullObject() const { return NULL; }
	virtual void			deleteObject(Row* p) const { delete p; }
	virtual void			initializeObject(Row* p) const { if (p) p->clear(); }
	virtual void			setObject(Row* dst, Row* src) const { assert(false); }
};
static Result::RowFactory	ROW_FACTORY;

/* QUERY-RESULT
 ******************************************************************/
Result::Result()
	: mRow(ROW_FACTORY)
{
}

Result::Result(const Result& o)
	: mRow(ROW_FACTORY)
{
	*this = o;
}

Result& Result::operator=(const Result& o)
{
	if (this != &o) {
		clear();
		mCol = o.mCol;
		addRows(o);
		mRequestTime = o.mRequestTime;
		mClientId = o.mClientId;
	}
	return *this;
}

void Result::clear()
{
	mCol.setSize(0);
	mColNames.clear();
	mRow.clear();
	mRequestTime = Poco::Timestamp(0);
	mClientId = 0;
}

bool Result::matches(const int* curType, ...) const
{
	bool			ans = true;
	va_list			ap;
	int				idx = 0;
	va_start(ap, curType);
	while (curType != NULL) {
		if (idx+1 > mCol.size() || mCol.data()[idx] != *curType) {
			ans = false;
			break;
		}
		curType = va_arg(ap, const int*);
		idx++;
	}
	va_end(ap);
	if (idx != mCol.size()) {
		ans = false;
#ifdef _DEBUG
		cout << "dbg QueryResult::matches() failed my size=" << mCol.size() << " idx=" << idx << endl;
#endif
	}
	return ans;
}

Poco::Timestamp Result::getRequestTime() const
{
	return mRequestTime;
}

int Result::getClientId() const
{
	return mClientId;
}

int Result::getColumnSize() const
{
	return mCol.size();
}

const std::string& Result::getColumnName(const int idx) const
{
	static const std::string		BLANK("");
	if (idx < 0 || idx >= mColNames.size()) return BLANK;
	return mColNames[idx];
}

bool Result::rowsAreEmpty() const
{
	return mRow.isEmpty();
}

int Result::getRowSize() const
{
	return mRow.size();
}

Result::RowIterator Result::getRows() const
{
	return RowIterator(*this);
}

bool Result::addRows(const Result& src)
{
	_ASSERT(mCol.size() == src.mCol.size());
	try {
		RowIterator			it(src);
		bool				ans = true;
		while (it.hasValue()) {
			const Row&		src = **(it.mRowIt);
			Row*			dst = mRow.pushBack();
			*dst = src;
			// Not completely safe but the odds of not being able to
			// allocate a few bytes for the ints and floats...  sheesh...
			if (dst->mString.size() != src.mString.size()) ans = false;
			++it;
		}
		return ans;
	} catch (std::exception&) {
	}
	return false;
}

void Result::popRowFront()
{
	if (!mRow.isEmpty()) mRow.popFront();
}

void Result::moveRow(Result& dst, const int from, const int to)
{
	mRow.move(dst.mRow, from, to);
}

void Result::swap(Result& o)
{
	mCol.swap(o.mCol);
	mRow.swap(o.mRow);
	std::swap(mRequestTime, o.mRequestTime);
	std::swap(mClientId, o.mClientId);
}

/* QUERY-RESULT::ROW
 ******************************************************************/
Result::Row::Row()
	: mString(STRING_FACTORY)
{
}

void Result::Row::clear()
{
	mName.clear();
	mNumeric.clear();
	mString.clear();
	mWString.clear();
}

void Result::Row::initialize(const int columns)
{
	if (columns > 0) {
		if (mNumeric.setSize(columns)) {
			double*			d = mNumeric.data();
			for (int k=0; k<columns; k++) d[k] = 0.0;
		}
		mNumeric.clear();
	}
}

Result::Row& Result::Row::operator=(const Row& o)
{
	if (this != &o) {
		mNumeric = o.mNumeric;
		mString.clear();
		try {
			StringIterator			it(o.mString);
			while (it.hasValue()) {
				const string*		src = *it;
				string*				dst = mString.pushBack();
				if (dst && src) *dst = *src;
				++it;
			}
		} catch (std::exception&) {
		}
		mWString = o.mWString;
	}
	return *this;
}

/* QUERY-RESULT::ROW-ITERATOR
 ******************************************************************/
Result::RowIterator::RowIterator(const RowIterator& o)
	: mRowIt(o.mRowIt)
{
}

Result::RowIterator::RowIterator(const Result& qr)
	: mRowIt(qr.mRow)
{
}

Result::RowIterator::RowIterator(const Result& qr, const std::string& str)
	: mRowIt(qr.mRow)
{
	while (mRowIt.hasValue() && (**mRowIt).mName != str) ++mRowIt;
}

Result::RowIterator& Result::RowIterator::operator=(const RowIterator& o)
{
	if (this != &o) mRowIt = o.mRowIt;
	return *this;
}

void Result::RowIterator::operator++()
{
	++mRowIt;
}

bool Result::RowIterator::hasValue() const
{
	return mRowIt.hasValue();
}

const std::string& Result::RowIterator::getName() const
{
	if (!mRowIt.hasValue()) return RESULT_EMPTY_STR;
	return (**mRowIt).mName;
}

// Surely somewhere in oF there's been a rounding function defined??  Well, use
// symmetric rounding, which I believe will be implemented in C+xx10
inline int query_round(const double d)
{
	return int(d > 0.0 ? floor(d + 0.5) : ceil(d - 0.5));
}

int Result::RowIterator::getInt(const int columnIndex) const
{
	const Row&		row = **mRowIt;
	if (row.mNumeric.size() <= columnIndex) return 0;
	return query_round(row.mNumeric.data()[columnIndex]);
}

float Result::RowIterator::getFloat(const int columnIndex) const
{
	const Row&		row = **mRowIt;
	if (row.mNumeric.size() <= columnIndex) return 0.0f;
	return (float)row.mNumeric.data()[columnIndex];
}

const std::string& Result::RowIterator::getString(const int columnIndex) const
{
	const Row&		row = **mRowIt;
	try {
		return *(row.mString.at(columnIndex));
	} catch (std::exception&) {
	}
	return RESULT_EMPTY_STR;
}

const std::wstring& Result::RowIterator::getWString(const int columnIndex) const
{
	const Row&		row = **mRowIt;
	if (columnIndex >= 0 && columnIndex < row.mWString.size()) return row.mWString[columnIndex];
	return RESULT_EMPTY_WSTR;
}

#ifdef _DEBUG
void Result::print() const
{
	cout << "QueryResult columnSize=" << mCol.size() << " rows=" << mRow.size() << endl;
	if (mCol.size() > 0) {
		cout << "\tcols ";
		for (int k=0; k<mCol.size(); k++) {
			const int	col = mCol.data()[k];
			if (col == QUERY_NUMERIC) cout << "NUMERIC ";
			else if (col == QUERY_STRING) cout << "STRING ";
			else cout << "? ";
		}
		cout << endl;
	}

	RowIterator				it(getRows());
	while (it.hasValue()) {
		for (int k=0; k<mCol.size(); k++) {
			const int		col = mCol.data()[k];
			if (col == QUERY_NUMERIC)		cout << "\t" << k << " = " << it.getFloat(k) << endl;
			else if (col == QUERY_STRING)	cout << "\t" << k << " = " << it.getString(k) << endl;
		}
		++it;
	}
}
#endif

} // namespace query

} // namespace ds
