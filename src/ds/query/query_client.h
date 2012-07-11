#pragma once
#ifndef DS_QUERY_QUERYCLIENT_H_
#define DS_QUERY_QUERYCLIENT_H_

#include <functional>
#include "ds/thread/work_client.h"
#include "ds/thread/work_request.h"
#include "ds/thread/work_request_list.h"
#include "ds/query/query_result.h"
#include "ds/query/query_talkback.h"

namespace ds {

namespace query {

/**
 * \class ds::query::Client
 * \brief Handle SQLite queries.
 */
class Client : public ds::WorkClient {
  public:			// static API
    // Supply this to query() flags to get a result that includes the names of the columns.
    static const int				INCLUDE_COLUMN_NAMES_F = (1<<0);

    // Run a synchronous query.
    static bool             query(const std::string& database, const std::string& query,
                                  Result&, const int flags = 0);
    // Run a synchronous query that opens the database in write mode.
    static bool             queryWrite(	const std::string& database, const std::string& query,
                                        Result&);

  public:
    Client(ui::SpriteEngine&, const std::function<void(const Result&, Talkback&)>& = nullptr);
	
    void                    setResultHandler(const std::function<void(const Result&, Talkback&)>&);

    // Start an asynchronous query, suppling the results to the resultHandler.
    // Clients can request the sendTime, which was supposed to be unique.  It's not though,
    // there are lots of cases where multiple queries will have the same time, so they can
    // also get the unique ID of this operation.
    bool                    runAsync(	const std::string& database, const std::string& query,
                                      Poco::Timestamp* sendTime = nullptr, int* id = nullptr);

  protected:
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
