#pragma once
#ifndef DS_QUERY_QUERYCLIENT_H_
#define DS_QUERY_QUERYCLIENT_H_

#include <functional>
#include "ds/thread/work_client.h"
#include "ds/thread/work_request_list.h"
#include "ds/query/query_result.h"
#include "ds/query/query_talkback.h"

namespace ds {

namespace query {

/**
 * \class ds::query::Client
 * \brief Handle SQLite queries. Use "query" in a thread to grab content from a sqlite database.
 */
class Client : public ds::WorkClient {
  public:			
	  

	/**
	* \brief INCLUDE_COLUMN_NAMES_F Supply this to query() flags to get a result that includes the names of the columns.
	*/
	static const int				INCLUDE_COLUMN_NAMES_F = (1<<0);

	
	/** \brief Run a synchronous query in read-only mode.
		\param database The filepath of the sqlite db to query
		\param query The string of the query statement to run on the db. E.g. "SELECT * FROM tablename"
		\param result The result of the query. See the Result class for information.
		\param flags Any flags to use when running the query, such as INCLUDE_COLUMN_NAMES_F
		\return True if the query was a success, and false if there was an issue (such as the db not found or an incorrectly formed query). If the query was a success and no rows were returned, true will be returned.
	*/
	static bool             query(const std::string& database, const std::string& query,
								  Result& result, const int flags = 0);

	/** \brief Run a synchronous query in write mode. Only use this method over "query()" if you need to commit something to the db.
		\param database The filepath of the sqlite db to query
		\param query The string of the query statement to run on the db. E.g. "SELECT * FROM tablename"
		\param result The result of the query. See the Result class for information.
		\return True if the query was a success, and false if there was an issue (such as the db not found or an incorrectly formed query).
	*/
	static bool             queryWrite(	const std::string& database, const std::string& query,
									   Result& result);

	/** \brief Regular constructor for non-static queries. In most cases, you can safely use the static API.	
	*/
	Client(ui::SpriteEngine&, const std::function<void(const Result&, Talkback&)>& = nullptr);
	
	/** \brief Set a handler lambda to handle results
		\param handlerFunction The lambda to be called when results are returned
	*/
	void                    setResultHandler(const std::function<void(const Result&, Talkback&)>& handlerFunction);

	/** \brief Start An asynchronous query, suppling the results to the resultHandler.
				Clients can request the sendTime, which was supposed to be unique.  It's not though,
				there are lots of cases where multiple queries will have the same time, so they can
				also get the unique ID of this operation.
		\param database The filepath of the sqlite db to query
		\param query The string of the query statement to run on the db. E.g. "SELECT * FROM tablename"
		\param sendTime The time the query was started?
		\param id
	*/
	bool                    runAsync(	const std::string& database, const std::string& query,
									  Poco::Timestamp* sendTime = nullptr, int* id = nullptr);

  protected:
	 /** 
	 */
	virtual void            handleResult(std::unique_ptr<WorkRequest>&);

  private:
	typedef ds::WorkClient  inherited;

	class Request : public ds::WorkRequest {
	  public:
		Request(const void* clientId);

		// input
		int                 mRunId;
		std::string         mDatabase,
							mQuery;

		// output
		ds::query::Result   mResult;
		ds::query::Talkback mTalkback;

		void                run();
	};

	ds::WorkRequestList<Request>
							mCache;

	int                     mRunId;

	std::function<void(const Result&, Talkback&)>
											  mResultHandler;
};

} // namespace query

} // namespace ds

#endif // DS_QUERY_QUERYCLIENT_H_
