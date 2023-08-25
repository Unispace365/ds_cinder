#include "stdafx.h"

#include "ds/query/sql_query_result_builder.h"

#include <iostream>

// using namespace std;

namespace ds { namespace query {

	/**
	 * \class SqlResultBuilder
	 */
	SqlResultBuilder::SqlResultBuilder(Result& qr, sqlite3_stmt* stmt)
	  : ResultBuilder(qr)
	  , mStatement(stmt)
	  , mStatementResult(SQLITE_ERROR) {
		next();
	}

	SqlResultBuilder::~SqlResultBuilder() {
		if (mStatement) sqlite3_finalize(mStatement);
	}

	int SqlResultBuilder::getColumnCount() const {
		if (mStatementResult != SQLITE_ROW) return 0;
		return sqlite3_data_count(mStatement);
	}

	int SqlResultBuilder::getColumnType(const int index) const {
		if (mStatementResult != SQLITE_ROW) return QUERY_NO_TYPE;
		const int ans = sqlite3_column_type(mStatement, index);
		// Don't allow int columns, since we can't choose at results-handling
		// time what type of data the results are, and we don't want to lose
		// any information.
		if (ans == SQLITE_INTEGER) return QUERY_NUMERIC;
		if (ans == SQLITE_FLOAT) return QUERY_NUMERIC;
		if (ans == SQLITE_TEXT) return QUERY_STRING;
		if (ans == SQLITE_NULL) return QUERY_NULL;
		return QUERY_NO_TYPE;
	}

	std::string SqlResultBuilder::getColumnName(const int index) {
		static const std::string BLANK("");

		if (mStatementResult != SQLITE_ROW) return BLANK;
		const char* n = sqlite3_column_name(mStatement, index);
		if (!n) return BLANK;
		return std::string(n);
	}

	bool SqlResultBuilder::hasNext() const {
		return mStatementResult == SQLITE_ROW;
	}

	bool SqlResultBuilder::next() {
		if (!mStatement) {
			mError = true;
			return false;
		}
		mStatementResult = sqlite3_step(mStatement);
		if (mStatementResult == SQLITE_ROW) return true;
		if (mStatementResult == SQLITE_BUSY) mError = true;
		return false;
	}

	bool SqlResultBuilder::getDouble(const int column, double& out) {
		if (mStatementResult != SQLITE_ROW) return false;
		out = sqlite3_column_double(mStatement, column);
		return true;
	}

	bool SqlResultBuilder::getString(const int column, std::string& out) {
		if (mStatementResult != SQLITE_ROW) return false;

		const unsigned char* ans = sqlite3_column_text(mStatement, column);
		if (ans == NULL)
			out.clear();
		else {
			// Clear out the buffer -- NOTE:  Some platforms are bugged and this
			// does not work.  Hopefully it's fine on any machine we use.
			mStrBuf.str("");
			mStrBuf << ans;
			out = mStrBuf.str();
		}
		return true;
	}

}} // namespace ds::query
