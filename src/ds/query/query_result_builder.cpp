#include "stdafx.h"

#include "ds/query/query_result_builder.h"

#include <iostream>
#include "ds/util/string_util.h"

//using namespace std;

namespace ds {

namespace query {

//static
void ResultBuilder::setRequestTime(Result& qr, const Poco::Timestamp& ts)
{
	qr.mRequestTime = ts;
}

void ResultBuilder::setClientId(Result& qr, const int id)
{
	qr.mClientId = id;
}

ResultBuilder::ResultBuilder(Result& qr)
	: mResult(qr)
	, mRow(NULL)
	, mColIdx(0)
	, mError(false)
{
	qr.clear();
}

ResultBuilder::~ResultBuilder()
{
}

bool ResultBuilder::isValid() const
{
	return !mError;
}

ResultBuilder& ResultBuilder::startRow()
{
	mRow = nullptr;
	try {
		mRow = mResult.pushBackRow();
		// Make sure the numbers are initialized to zero.  This is critical because of
		// the design of SQLite -- any numeric fields with NULL values show up as text
		// fields, but if the client is expecting a number, we want to default to zero
		// still, not whatever happened to be there.
		mRow->initialize(static_cast<int>(mResult.mCol.size()));
		mColIdx = 0;
	} catch (std::exception&) {
		mError = true;
	}
	return *this;
}

ResultBuilder& ResultBuilder::addNumeric(const double v)
{
	if (mError || !mRow) return *this;
	const int			at = mColIdx;
	mColIdx++;

	mRow->mNumeric.resize(mColIdx);
	mRow->mNumeric[at] = v;
	return *this;
}

ResultBuilder& ResultBuilder::addString(const std::string& v)
{
	if (mError || !mRow) return *this;
	const int			at = mColIdx;
	mColIdx++;

	try {
		// First the utf-8 string.  Ideally this goes away
		while (mRow->mString.size() < mColIdx) mRow->mString.push_back("");
		mRow->mString[at] = v;
		// Now the wstring
		while (mRow->mWString.size() < mColIdx) mRow->mWString.push_back(L"");
		mRow->mWString[at] = ds::wstr_from_utf8(v);
	} catch (std::exception&) {
		mError = true;
	}

	return *this;
}

void ResultBuilder::build(const bool columnNames)
{
	if (mError==true) return;

	// Set up the columns
	try {
		const int	count = getColumnCount();
		if (count < 1) {
			// I think a count of 0 isn't technically an error, because the result set might just be empty.
			if (count < 0) mError = true;
			return;
		}
		for (int k=0; k<count; ++k) {
			const int	ct = getColumnType(k);
			if(ct == QUERY_NO_TYPE){
				mError = true;
			} else {
				mResult.mCol.push_back(ct);
			}
			if (columnNames) {
				mResult.mColNames.push_back(getColumnName(k));
			}
		}
	} catch (std::exception&) {
		mError = true;
		return;
	}

	// Empty results are considered valid.
	if (!hasNext()) return;

	if (mError == true || mResult.mCol.size() < 1) {
		mError = true;
		return;
	}

	// Read the rows
	while (hasNext()) {
		startRow();
		for (int k=0; k<mResult.mCol.size(); k++) {
			const int		col = mResult.mCol.data()[k];
			if (col == QUERY_NUMERIC) {
				double		v = 0.0f;
				if (!getDouble(k, v)) mError = true;
				addNumeric(v);
			} else if (col == QUERY_STRING) {
				std::string		v;
				if (!getString(k, v)) mError = true;
				addString(v);
			} else if (col == QUERY_NULL) {
				// I can't determine at any point the actual type of the column,
				// because it looks like sqlite always bases that info on the first
				// row in the result set. So in this case, I've got to just get
				// every type.
				double		v = 0.0f;
				if (!getDouble(k, v)) v = 0.0f;
				addNumeric(v);

				--mColIdx;
				std::string		str;
				getString(k, str);
				addString(str);
			}
		}
		next();
	}
}

} // namespace query

} // namespace ds
