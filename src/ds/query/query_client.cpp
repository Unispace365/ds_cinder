#include "ds/query/query_client.h"

#include "ds/debug/debug_defines.h"
#include "ds/util/memory_ds.h"
#include "ds/thread/work_manager.h"
#include "ds/query/sql_database.h"
#include "ds/query/sql_query_result_builder.h"

static bool run_query(ds::query::SqlDatabase& db,  const std::string& select, ds::query::Result& qr, const int flags = 0)
{
	qr.clear();
	if (select.empty()) return false;

	ds::query::SqlResultBuilder		qrb(qr, db.rawSelect(select));
	qrb.build((flags&ds::query::Client::INCLUDE_COLUMN_NAMES_F) != 0);
	return qrb.isValid();
}

namespace ds {

namespace query {

/**
 * \class ds::query::Client static
 */
bool Client::query(	const std::string& database, const std::string& select,
					          Result& qr, const int flags)
{
	qr.clear();
	if (database.empty() || select.empty()) return false;

	int							errorCode = 0;
	SqlDatabase					sqlDb(database, SQLITE_OPEN_READONLY, &errorCode);
	if (errorCode != SQLITE_OK) return false;
	return run_query(sqlDb, select, qr, flags);
}

bool Client::queryWrite(const std::string& database, const std::string& select,
						            Result& qr)
{
	qr.clear();
	if (database.empty()) return false;

	int							errorCode = 0;
	SqlDatabase					sqlDb(database, SQLITE_OPEN_READWRITE, &errorCode);
	if (errorCode != SQLITE_OK) return false;
	return run_query(sqlDb, select, qr);
}

/**
 * \class ds::query::Client
 */
Client::Client(ui::SpriteEngine& e, const std::function<void(const Result&, Talkback&)>& h)
	: inherited(e)
	, mCache(this)
	, mRunId(0)
	, mResultHandler(h)
{
}

void Client::setResultHandler(const std::function<void(const Result&, Talkback&)>& h)
{
	mResultHandler = h;
}

bool Client::runAsync(	const std::string& database, const std::string& query,
						Poco::Timestamp* sendTime, int* id)
{
	if (database.empty() || query.empty()) {
		DS_DBG_CODE(std::cout << "ERROR ds::query::Client() empty value database=" << database << " query=" << query << std::endl);
		return false;
	}

	std::unique_ptr<Request>		r(std::move(mCache.next()));
	if (!r) return false;

	r->mRunId = (mRunId++);
	r->mDatabase = database;
	r->mQuery = query;
	r->mResult.clear();
	r->mTalkback.clear();
	if (id) *id = r->mRunId;
	return mManager.sendRequest(ds::unique_dynamic_cast<WorkRequest, Request>(r), sendTime);
}

void Client::handleResult(std::unique_ptr<WorkRequest>& wr)
{
	std::unique_ptr<Request>		r(ds::unique_dynamic_cast<Request, WorkRequest>(wr));
	if (!r) return;

	if (mResultHandler) mResultHandler(r->mResult, r->mTalkback);
	// Recycle the request.
	mCache.push(r);
}

/**
 * \class ds::QueryClient::Request
 */
Client::Request::Request(const void* clientId)
	: WorkRequest(clientId)
	, mRunId(0)
{
	mDatabase.reserve(128);
	mQuery.reserve(256);
}

void Client::Request::run()
{
	int							errorCode = 0;
	SqlDatabase					resourceDB(mDatabase, SQLITE_OPEN_READONLY, &errorCode);
	if (errorCode != SQLITE_OK) {
		DS_DBG_CODE(std::cout << "ds::query::Client::Request: Unable to access the resource database (SQLite error " << errorCode << ").");
	} else {
		SqlResultBuilder		qrb(mResult, resourceDB.rawSelect(mQuery));
		qrb.build();

		ResultBuilder::setRequestTime(mResult, mRequestTime);
		ResultBuilder::setClientId(mResult, mRunId);
	}
}

} // namespace query

} // namespace ds
