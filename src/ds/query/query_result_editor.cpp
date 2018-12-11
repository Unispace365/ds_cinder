#include "stdafx.h"

#include "ds/query/query_result_editor.h"

#include <iostream>
#include "ds/util/string_util.h"

namespace ds {
namespace query {

/**
 * \class ResultEditor
 */
ResultEditor::ResultEditor(Result& qr, const bool append)
		: mResult(qr)
		, mRow(NULL)
		, mColIdx(0)
		, mError(false) {
	if(!append) qr.clear();
}

bool ResultEditor::isValid() const {
	return !mError;
}

ResultEditor& ResultEditor::setColumnType(const size_t index, const int type) {
	while (mResult.mCol.size() <= index) mResult.mCol.push_back(QUERY_NO_TYPE);
	mResult.mCol[index] = type;
	return *this;
}

ResultEditor& ResultEditor::setColumnName(const size_t index, const std::string& name) {
	while (mResult.mColNames.size() <= index) mResult.mColNames.push_back(std::string());
	mResult.mColNames[index] = name;
	return *this;
}

ResultEditor& ResultEditor::setColumn(const size_t index, const int type, const std::string& name) {
	setColumnType(index, type);
	setColumnName(index, name);
	return *this;
}

ResultEditor& ResultEditor::startRow() {
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

ResultEditor& ResultEditor::addNumeric(const double v)
{
	if (mError || !mRow) return *this;
	const int			at = mColIdx;
	mColIdx++;

	mRow->mNumeric.resize(mColIdx);
	mRow->mNumeric[at] = v;
	return *this;
}

ResultEditor& ResultEditor::addString(const std::wstring& v)
{
	if (mError || !mRow) return *this;
	const int			at = mColIdx;
	mColIdx++;

	try {
		// First the utf-8 string.  Ideally this goes away
		while (mRow->mString.size() < mColIdx) mRow->mString.push_back("");
		mRow->mString[at] = ds::utf8_from_wstr(v);
		// Now the wstring
		while (mRow->mWString.size() < mColIdx) mRow->mWString.push_back(L"");
		mRow->mWString[at] = v;
	} catch (std::exception&) {
		mError = true;
	}

	return *this;
}

} // namespace query
} // namespace ds
