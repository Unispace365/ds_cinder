#include "ds/query/query_result_builder.h"

#include <iostream>
#include "ds/util/string_util.h"

using namespace std;

namespace ds {

namespace query {

/**
 * \class ds::query::ResultBuilder static
 */
void ResultBuilder::setRequestTime(Result& qr, const Poco::Timestamp& ts)
{
	qr.mRequestTime = ts;
}

void ResultBuilder::setClientId(Result& qr, const int id)
{
	qr.mClientId = id;
}

/**
 * \class ds::query::ResultBuilder
 */
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
		mRow = mResult.mRow.pushBack();
		// Make sure the numbers are initialized to zero.  This is critical because of
		// the design of SQLite -- any numeric fields with NULL values show up as text
		// fields, but if the client is expecting a number, we want to default to zero
		// still, not whatever happened to be there.
		mRow->initialize(mResult.mCol.size());
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

	if (!mRow->mNumeric.setSize(mColIdx)) mError = true;
	else mRow->mNumeric.data()[at] = v;
	return *this;
}

ResultBuilder& ResultBuilder::addString(const std::string& v)
{
	if (mError || !mRow) return *this;
	const int			at = mColIdx;
	mColIdx++;

	try {
		// First the utf-8 string.  Ideally this goes away
		while (mRow->mString.size() < mColIdx) mRow->mString.pushBack();
		*(mRow->mString.at(at)) = v;
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
		int			ct;
		for (int k=0; ((ct = getColumnType(k)) != QUERY_NO_TYPE); k++) {
			if (!mResult.mCol.add(ct)) mError = true;
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
				string		v;
				if (!getString(k, v)) mError = true;
				addString(v);
			}
		}
		next();
	}
}

} // namespace query

} // namespace ds
