#include "stdafx.h"

#include "ds/query/query_result.h"

#include <Poco/DateTimeParser.h>
#include <algorithm>
#include <ds/util/string_util.h>
#include <iostream>
#include <math.h>
#include <stdarg.h>

// using namespace std;

namespace {
const std::string  RESULT_EMPTY_STR("");
const std::wstring RESULT_EMPTY_WSTR(L"");
} // namespace

#ifndef WIN32
#define _ASSERT assert
#endif

namespace ds { namespace query {

	const int QUERY_NO_TYPE = 0;
	const int QUERY_STRING	= 1;
	const int QUERY_NUMERIC = 2;
	const int QUERY_NULL	= 3;

	/* QUERY-RESULT
	 ******************************************************************/
	Result::Result() {}

	Result::Result(const Result& o) {
		*this = o;
	}

	Result& Result::operator=(const Result& o) {
		if (this != &o) {
			clear();
			mCol = o.mCol;
			addRows(o);
			mRequestTime = o.mRequestTime;
			mClientId	 = o.mClientId;
		}
		return *this;
	}

	Result& Result::operator=(const RowIterator& it) {
		clear();
		mCol		 = it.mResult.mCol;
		mColNames	 = it.mResult.mColNames;
		mRequestTime = it.mResult.mRequestTime;
		mClientId	 = it.mResult.mClientId;

		try {
			if (it.mRowIt != it.mResult.mRow.end()) {
				const Row* src = it.mRowIt->get();
				if (src) {
					Result::Row* dst = pushBackRow();
					*dst			 = *src;
				}
			}
		} catch (std::exception const&) {}

		return *this;
	}

	void Result::clear() {
		mCol.clear();
		mColNames.clear();
		mRow.clear();
		mRequestTime = Poco::Timestamp(0);
		mClientId	 = 0;
	}

	bool Result::matches(const int* curType, ...) const {
		bool	ans = true;
		va_list ap;
		int		idx = 0;
		va_start(ap, curType);
		while (curType != NULL) {
			if (idx >= mCol.size() || mCol[idx] != *curType) {
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
			std::cout << "dbg QueryResult::matches() failed my size=" << mCol.size() << " idx=" << idx << std::endl;
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
		return static_cast<int>(mCol.size());
	}

	const int Result::getColumnType(const int idx) const {
		if (idx < 0 || idx >= mColNames.size()) return QUERY_NO_TYPE;
		return mCol[idx];
	}

	const std::string& Result::getColumnName(const int idx) const {
		static const std::string BLANK("");
		if (idx < 0 || idx >= mColNames.size()) return BLANK;
		return mColNames[idx];
	}

	bool Result::rowsAreEmpty() const {
		return mRow.empty();
	}

	int Result::getRowSize() const {
		return static_cast<int>(mRow.size());
	}

	Result::RowIterator Result::getRows() const {
		return RowIterator(*this);
	}

	Result::RowIterator Result::rowAt(const size_t index) const {
		return RowIterator(*this, index);
	}

	bool Result::addRows(const Result& src) {
		_ASSERT(mCol.size() == src.mCol.size());
		try {
			RowIterator it(src);
			bool		ans = true;
			while (it.hasValue()) {
				const Row& src = **(it.mRowIt);
				Row*	   dst = pushBackRow();
				*dst		   = src;
				// Not completely safe but the odds of not being able to
				// allocate a few bytes for the ints and floats...  sheesh...
				if (dst->mString.size() != src.mString.size()) ans = false;
				++it;
			}
			return ans;
		} catch (std::exception&) {}
		return false;
	}

	void Result::popRowFront() {
		if (!mRow.empty()) {
			mRow.erase(mRow.begin());
		}
	}

	void Result::swap(Result& o) {
		mCol.swap(o.mCol);
		mRow.swap(o.mRow);
		std::swap(mRequestTime, o.mRequestTime);
		std::swap(mClientId, o.mClientId);
	}

	void Result::sortByString(const int																 columnIndex,
							  const std::function<bool(const std::string& a, const std::string& b)>& clientFn) {
		if (!clientFn) return;

		auto fn = [columnIndex, &clientFn](const std::unique_ptr<Row>& a, const std::unique_ptr<Row>& b) -> bool {
			const size_t ci(static_cast<size_t>(columnIndex));
			const Row*	 ar = a.get();
			const Row*	 br = b.get();
			if (!ar || !br) return false;
			if (ar->mString.size() <= ci || br->mString.size() <= ci) return false;
			return clientFn(ar->mString[ci], br->mString[ci]);
		};

		std::sort(mRow.begin(), mRow.end(), fn);
	}

	void Result::sort_if(const std::function<bool(const RowIterator& a, const RowIterator& b)>& clientFn) {
		if (!clientFn) return;

		auto fn = [this, &clientFn](const std::unique_ptr<Row>& _a, const std::unique_ptr<Row>& _b) -> bool {
			const RowIterator a(*this, _a), b(*this, _b);
			if (!a.hasValue() || !b.hasValue()) {
				return false;
			}
			return clientFn(a, b);
		};

		std::sort(mRow.begin(), mRow.end(), fn);
	}

	Result::Row* Result::pushBackRow() {
		mRow.push_back(std::move(std::unique_ptr<Row>(new Row())));
		return mRow.back().get();
	}

	/* QUERY-RESULT::ROW
	 ******************************************************************/
	Result::Row::Row() {}

	void Result::Row::clear() {
		mName.clear();
		mNumeric.clear();
		mString.clear();
		mWString.clear();
	}

	void Result::Row::initialize(const int columns) {
		if (columns > 0) {
			mNumeric.resize(columns, 0.0);
			mNumeric.clear();
		}
	}

	Result::Row& Result::Row::operator=(const Row& o) {
		if (this != &o) {
			try {
				mName	 = o.mName;
				mNumeric = o.mNumeric;
				mString	 = o.mString;
				mWString = o.mWString;
			} catch (std::exception&) {}
		}
		return *this;
	}

	/* QUERY-RESULT::ROW-ITERATOR
	 ******************************************************************/
	Result::RowIterator::RowIterator(const RowIterator& o)
	  : mResult(o.mResult)
	  , mRowIt(o.mRowIt)
	  , mOverride(nullptr) {}

	Result::RowIterator::RowIterator(const Result& qr)
	  : mResult(qr)
	  , mRowIt(qr.mRow.begin())
	  , mOverride(nullptr) {}

	Result::RowIterator::RowIterator(const Result& qr, const std::string& str)
	  : mResult(qr)
	  , mRowIt(qr.mRow.begin())
	  , mOverride(nullptr) {
		while (mRowIt != mResult.mRow.end()) {
			if (mRowIt->get()->mName == str) break;
			++mRowIt;
		}
	}

	Result::RowIterator::RowIterator(const Result& qr, const size_t index)
	  : mResult(qr)
	  , mRowIt(qr.mRow.end())
	  , mOverride(nullptr) {
		if (index < qr.mRow.size()) mRowIt = qr.mRow.begin() + index;
	}

	Result::RowIterator::RowIterator(const Result& qr, const std::unique_ptr<Row>& r)
	  : mResult(qr)
	  , mRowIt(qr.mRow.end())
	  , mOverride(r.get()) {}

	void Result::RowIterator::operator++() {
		if (mOverride) {
			mOverride = nullptr;
		} else {
			++mRowIt;
		}
	}

	void Result::RowIterator::operator+=(const int count) {
		if (!mOverride) mRowIt += count;
	}

	bool Result::RowIterator::hasValue() const {
		if (mOverride) return true;
		return mRowIt != mResult.mRow.end() && mRowIt->get() != nullptr;
	}

	const std::string& Result::RowIterator::getName() const {
		if (mOverride) {
			return mOverride->mName;
		}
		if (!hasValue()) return RESULT_EMPTY_STR;
		return (**mRowIt).mName;
	}

	// Surely somewhere in oF there's been a rounding function defined??  Well, use
	// symmetric rounding, which I believe will be implemented in C+xx10
	inline int query_round(const double d) {
		return int(d > 0.0 ? floor(d + 0.5) : ceil(d - 0.5));
	}

	inline int64_t query_round_64(const double d) {
		return int64_t(d > 0.0 ? floor(d + 0.5) : ceil(d - 0.5));
	}

	int Result::RowIterator::getInt(const int columnIndex) const {
		if (columnIndex < 0) return 0;
		const Row& row = (mOverride ? *mOverride : **mRowIt);
		// Deal with the case where the column got misinterpreted as a string --
		// this can happen when there's a NULL in the data set.
		if (columnIndex < row.mWString.size() && !row.mWString[columnIndex].empty()) {
			int ans = 0;
			if (ds::wstring_to_value(row.mWString[columnIndex], ans)) {
				return ans;
			}
		}
		if (row.mNumeric.size() <= columnIndex) return 0;
		return query_round(row.mNumeric[columnIndex]);
	}

	int64_t Result::RowIterator::getInt64(const int columnIndex) const {
		if (columnIndex < 0) return 0;
		const Row& row = (mOverride ? *mOverride : **mRowIt);
		// Deal with the case where the column got misinterpreted as a string --
		// this can happen when there's a NULL in the data set.
		if (columnIndex < row.mWString.size() && !row.mWString[columnIndex].empty()) {
			int64_t ans = 0;
			if (ds::wstring_to_value(row.mWString[columnIndex], ans)) {
				return ans;
			}
		}
		if (row.mNumeric.size() <= columnIndex) return 0;
		return query_round_64(row.mNumeric[columnIndex]);
	}

	float Result::RowIterator::getFloat(const int columnIndex) const {
		if (columnIndex < 0) return 0;
		const Row& row = (mOverride ? *mOverride : **mRowIt);
		// Deal with the case where the column got misinterpreted as a string --
		// this can happen when there's a NULL in the data set.
		if (columnIndex < row.mWString.size() && !row.mWString[columnIndex].empty()) {
			float ans = 0.0f;
			if (ds::wstring_to_value(row.mWString[columnIndex], ans)) {
				return ans;
			}
		}
		if (row.mNumeric.size() <= columnIndex) return 0.0f;
		return static_cast<float>(row.mNumeric[columnIndex]);
	}

	const std::string& Result::RowIterator::getString(const int columnIndex) const {
		const Row& row = (mOverride ? *mOverride : **mRowIt);
		if (columnIndex >= 0 && columnIndex < row.mString.size()) return row.mString[columnIndex];
		return RESULT_EMPTY_STR;
	}

	const std::wstring& Result::RowIterator::getWString(const int columnIndex) const {
		const Row& row = (mOverride ? *mOverride : **mRowIt);
		if (columnIndex >= 0 && columnIndex < row.mWString.size()) return row.mWString[columnIndex];
		return RESULT_EMPTY_WSTR;
	}

	const Poco::DateTime Result::RowIterator::getDateTime(const int columnIndex) const {
		const Row& row = (mOverride ? *mOverride : **mRowIt);
		if (columnIndex >= 0 && columnIndex < row.mString.size()) {
			try {
				std::string	   stringToParse = row.mString[columnIndex];
				int			   tzd			 = 0;
				Poco::DateTime output;
				Poco::DateTimeParser::parse("%Y-%o-%d %h:%M:%S %a", stringToParse, output, tzd);
				return output;
			} catch (...) {}
		}
		return Poco::DateTime();
	}

	const Poco::DateTime Result::RowIterator::getDateTime24hr(const int columnIndex) const {
		const Row& row = (mOverride ? *mOverride : **mRowIt);
		if (columnIndex >= 0 && columnIndex < row.mString.size()) {
			try {
				std::string	   stringToParse = row.mString[columnIndex];
				int			   tzd			 = 0;
				Poco::DateTime output;
				Poco::DateTimeParser::parse("%Y-%m-%d %H:%M:%S", stringToParse, output, tzd);
				return output;
			} catch (...) {}
		}
		return Poco::DateTime();
	}

#ifdef _DEBUG
	void Result::print() const {
		std::cout << "QueryResult columnSize=" << mCol.size() << " rows=" << mRow.size() << std::endl;
		if (mCol.size() > 0) {
			std::cout << "\tcols ";
			for (auto it = mCol.begin(), end = mCol.end(); it != end; ++it) {
				const int col(*it);
				if (col == QUERY_NUMERIC)
					std::cout << "NUMERIC ";
				else if (col == QUERY_STRING)
					std::cout << "STRING ";
				else
					std::cout << "? ";
			}
			std::cout << std::endl;
		}

		RowIterator it(getRows());
		while (it.hasValue()) {
			for (int k = 0; k < static_cast<int>(mCol.size()); ++k) {
				const int col = mCol[k];
				if (col == QUERY_NUMERIC)
					std::cout << "\t" << k << " = " << it.getFloat(k) << std::endl;
				else if (col == QUERY_STRING)
					std::cout << "\t" << k << " = " << it.getString(k) << std::endl;
			}
			++it;
		}
	}
#endif

}} // namespace ds::query
