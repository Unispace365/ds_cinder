#pragma once
#ifndef DS_QUERY_SQLDATABASE_H_
#define DS_QUERY_SQLDATABASE_H_

#include "ds/query/sqlite/sqlite3.h"
#include <sstream>

namespace ds { namespace query {

	/**
	 * \class SqlDatabase
	 * \brief Handle an sqlite database connection.
	 */
	class SqlDatabase {
	  public:
		SqlDatabase(const std::string& sDB, int flags, int* errorCode);
		~SqlDatabase();

		/// Answer a hook to process a query results.  This could be
		/// cleaner, it started off as a modification to the ofx stuff.
		/// Client is responsible for finalizing the statement.
		sqlite3_stmt* rawSelect(const std::string& rawSqlSelect);

	  private:
		sqlite3*	db;
		std::string db_file;
	};

}} // namespace ds::query

#endif // DS_QUERY_SQLDATABASE_H_
