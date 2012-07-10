#pragma once
#ifndef DS_QUERY_SQLQUERYRESULTBUILDER_H_
#define DS_QUERY_SQLQUERYRESULTBUILDER_H_

#include <sstream>
#include "ds/query/sqlite/sqlite3.h"
#include "ds/query/query_result_builder.h"

namespace ds {

namespace query {

/**
 * \class ds::query::SqlResultBuilder
 * \brief Translate an SQL statement into a local result.
 */
class SqlResultBuilder : public ResultBuilder
{
public:
	SqlResultBuilder(Result&, sqlite3_stmt* = nullptr);
	virtual ~SqlResultBuilder();

	virtual int					getColumnType(const int index) const;
	virtual std::string			getColumnName(const int index);
	virtual bool				hasNext() const;
	virtual bool				next();
	virtual bool				getDouble(const int column, double&);
	virtual bool				getString(const int column, std::string&);

private:
	sqlite3_stmt*				mStatement;
	int							mStatementResult;
	// Reuse our string buffer
	std::stringstream			mStrBuf;
};

} // namespace query

} // namespace ds

#endif // DS_QUERY_SQLQUERYRESULTBUILDER_H_