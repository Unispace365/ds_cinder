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

/* QUERY-RESULT
 ******************************************************************/
Result::Result() {
}

Result::Result(const Result& o) {
	*this = o;
}

Result& Result::operator=(const Result& o) {
	if (this != &o) {
		clear();
		mCol = o.mCol;
		addRows(o);
		mRequestTime = o.mRequestTime;
		mClientId = o.mClientId;
	}
	return *this;
}

Result& Result::operator=(const RowIterator& it) {
	clear();
	mCol = it.mResult.mCol;
	mColNames = it.mResult.mColNames;
	mRequestTime = it.mResult.mRequestTime;
	mClientId = it.mResult.mClientId;

	try {
		if (it.mRowIt != it.mResult.mRow.end()) {
			const Row*			src = it.mRowIt->get();
			if (src) {
				Result::Row*	dst = pushBackRow();
				*dst = *src;
			}
		}
	} catch (std::exception const&) {
	}

	return *this;
}

void Result::clear() {
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

Poco::Timestamp Result::getRequestTime() const {
	return mRequestTime;
}

int Result::getClientId() const {
	return mClientId;
}

int Result::getColumnSize() const {
	return mCol.size();
}

const std::string& Result::getColumnName(const int idx) const {
	static const std::string		BLANK("");
	if (idx < 0 || idx >= mColNames.size()) return BLANK;
	return mColNames[idx];
}

bool Result::rowsAreEmpty() const {
	return mRow.empty();
}

int Result::getRowSize() const {
	return mRow.size();
}

Result::RowIterator Result::getRows() const {
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
			Row*			dst = pushBackRow();
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

void Result::popRowFront() {
	if (!mRow.empty()) {
		mRow.erase(mRow.begin());
	}
}

#if 0
void Result::moveRow(Result& dst, const int from, const int to)
{
	mRow.move(dst.mRow, from, to);
}
#endif

void Result::swap(Result& o)  {
	mCol.swap(o.mCol);
	mRow.swap(o.mRow);
	std::swap(mRequestTime, o.mRequestTime);
	std::swap(mClientId, o.mClientId);
}

Result::Row* Result::pushBackRow() {
	mRow.push_back(std::move(std::unique_ptr<Row>(new Row())));
	return mRow.back().get();
}

/* QUERY-RESULT::ROW
 ******************************************************************/
Result::Row::Row() {
}

void Result::Row::clear() {
	mName.clear();
	mNumeric.clear();
	mString.clear();
	mWString.clear();
}

void Result::Row::initialize(const int columns) {
	if (columns > 0) {
		if (mNumeric.setSize(columns)) {
			double*			d = mNumeric.data();
			for (int k=0; k<columns; k++) d[k] = 0.0;
		}
		mNumeric.clear();
	}
}

Result::Row& Result::Row::operator=(const Row& o) {
	if (this != &o) {
		try {
			mName = o.mName;
			mNumeric = o.mNumeric;
			mString = o.mString;
			mWString = o.mWString;
		} catch (std::exception&) {
		}
	}
	return *this;
}

/* QUERY-RESULT::ROW-ITERATOR
 ******************************************************************/
Result::RowIterator::RowIterator(const RowIterator& o)
		: mResult(o.mResult)
		, mRowIt(o.mRowIt) {
}

Result::RowIterator::RowIterator(const Result& qr)
		: mResult(qr)
		, mRowIt(qr.mRow.begin()) {
}

Result::RowIterator::RowIterator(const Result& qr, const std::string& str)
		: mResult(qr)
		, mRowIt(qr.mRow.begin()) {
	while (mRowIt != mResult.mRow.end()) {
		if (mRowIt->get()->mName == str) break;
		++mRowIt;
	}
}

#if 0
Result::RowIterator& Result::RowIterator::operator=(const RowIterator& o)
{
	if (this != &o) {
		mResult = o.mResult;
		mRowIt = o.mRowIt;
	}
	return *this;
}
#endif

void Result::RowIterator::operator++() {
	++mRowIt;
}

void Result::RowIterator::operator+=(const int count) {
	mRowIt += count;
}

bool Result::RowIterator::hasValue() const {
	return mRowIt != mResult.mRow.end() && mRowIt->get() != nullptr;
}

const std::string& Result::RowIterator::getName() const {
	if (!hasValue()) return RESULT_EMPTY_STR;
	return (**mRowIt).mName;
}

// Surely somewhere in oF there's been a rounding function defined??  Well, use
// symmetric rounding, which I believe will be implemented in C+xx10
inline int query_round(const double d) {
	return int(d > 0.0 ? floor(d + 0.5) : ceil(d - 0.5));
}

int Result::RowIterator::getInt(const int columnIndex) const {
	const Row&		row = **mRowIt;
	if (row.mNumeric.size() <= columnIndex) return 0;
	return query_round(row.mNumeric.data()[columnIndex]);
}

float Result::RowIterator::getFloat(const int columnIndex) const {
	const Row&		row = **mRowIt;
	if (row.mNumeric.size() <= columnIndex) return 0.0f;
	return (float)row.mNumeric.data()[columnIndex];
}

const std::string& Result::RowIterator::getString(const int columnIndex) const {
	const Row&		row = **mRowIt;
	if (columnIndex >= 0 && columnIndex < row.mString.size()) return row.mString[columnIndex];
	return RESULT_EMPTY_STR;
}

const std::wstring& Result::RowIterator::getWString(const int columnIndex) const {
	const Row&		row = **mRowIt;
	if (columnIndex >= 0 && columnIndex < row.mWString.size()) return row.mWString[columnIndex];
	return RESULT_EMPTY_WSTR;
}

#ifdef _DEBUG
void Result::print() const {
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
