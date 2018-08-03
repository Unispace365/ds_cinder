#pragma once
#ifndef DS_QUERY_QUERYRESULTEDITOR_H_
#define DS_QUERY_QUERYRESULTEDITOR_H_

#include <string>
#include "ds/query/query_result.h"

namespace ds {
namespace query {

/**
 * \class ResultEditor
 * \brief An editor on a query Result.
 */
class ResultEditor
{
public:
	ResultEditor(Result&, const bool append=false);

	bool						isValid() const;

	ResultEditor&				setColumnType(const size_t index, const int type);
	ResultEditor&				setColumnName(const size_t index, const std::string& name);
	ResultEditor&				setColumn(const size_t index, const int type, const std::string& name);

	ResultEditor&				startRow();
	ResultEditor&				addNumeric(const double);
	ResultEditor&				addString(const std::wstring&);

private:
	Result&						mResult;
	Result::Row*				mRow;
	int							mColIdx;
	bool						mError;
};

} // namespace query
} // namespace ds

#endif // DS_QUERY_QUERYRESULTEDITOR_H_
