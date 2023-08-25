#pragma once
#ifndef DS_QUERY_SQLQUERYRESULTBUILDER_H_
#define DS_QUERY_SQLQUERYRESULTBUILDER_H_

#include "ds/query/query_result_builder.h"
#include "ds/query/sqlite/sqlite3.h"
#include <sstream>

namespace ds { namespace query {

	/**
	 * \class SqlResultBuilder
	 * \brief Translate an SQL statement into a local result.
	 */
	class SqlResultBuilder : public ResultBuilder {
	  public:
		SqlResultBuilder(Result&, sqlite3_stmt* = nullptr);
		virtual ~SqlResultBuilder();

		virtual int			getColumnCount() const;
		virtual int			getColumnType(const int index) const;
		virtual std::string getColumnName(const int index);
		virtual bool		hasNext() const;
		virtual bool		next();
		virtual bool		getDouble(const int column, double&);
		virtual bool		getString(const int column, std::string&);

	  private:
		sqlite3_stmt* mStatement;
		int			  mStatementResult;
		/// Reuse our string buffer
		std::stringstream mStrBuf;
	};

}} // namespace ds::query

#endif // DS_QUERY_SQLQUERYRESULTBUILDER_H_
