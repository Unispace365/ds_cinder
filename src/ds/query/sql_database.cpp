#include "ds/query/sql_database.h"

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
		stringstream s;
		s << "SqlDatabase: Unable to access the database " << sDB << " (SQLite error " << result << ")." << endl;
#ifdef _DEBUG
		cout << s.str();
		Poco::Thread::sleep(1000*10);
#endif
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
#ifdef _DEBUG
		stringstream	s;
		s << "SqlDatabase::rawSelect SQL error = " << err << " on select=" << rawSqlSelect << endl;
		cout << s.str() << endl;
#endif
		return NULL;
	}
	return statement;
}

} // namespace query

} // namespace ds
