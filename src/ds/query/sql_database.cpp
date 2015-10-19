#include "ds/query/sql_database.h"
#include "ds/debug/logger.h"

#include <iostream>
#include <Poco/Thread.h>

using namespace std;

namespace ds {

namespace query {

/* SQL-DATABASE
 ******************************************************************/
SqlDatabase::SqlDatabase(const std::string& sDB, int flags, int *errorCode)
	: db(NULL)
	, db_file(sDB)
{
	const int		result = sqlite3_open_v2(db_file.c_str(), &db, flags, 0);
	if (errorCode) *errorCode = result;
	if (result == SQLITE_OK) {
		sqlite3_busy_timeout(db, 1500);
	} else {
		// Actually a fatal error but ...
		DS_LOG_ERROR("  SqlDatabase: Unable to access the database " << sDB << " (SQLite error " << result << ")." << endl);
		
		// Why were we living with 10 seconds of sleep for so long?
		// Leaving this here for future people to ponder their existence
		//Poco::Thread::sleep(1000*10);
	}
}

SqlDatabase::~SqlDatabase()
{
	sqlite3_close(db);
}

sqlite3_stmt* SqlDatabase::rawSelect(const std::string& rawSqlSelect)
{
	sqlite3_stmt*		statement;
	const int			err = sqlite3_prepare_v2(db, rawSqlSelect.c_str(), -1, &statement, 0);
	if (err != SQLITE_OK) {
		sqlite3_finalize(statement);
		DS_LOG_ERROR( "SqlDatabase::rawSelect SQL error = " << err << " on select=" << rawSqlSelect << endl);
		return NULL;
	}
	return statement;
}

} // namespace query

} // namespace ds
