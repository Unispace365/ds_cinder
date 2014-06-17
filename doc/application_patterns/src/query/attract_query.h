#pragma once
#ifndef NA_QUERY_ATTRACTQUERY_H_
#define NA_QUERY_ATTRACTQUERY_H_

#include <functional>
#include <Poco/Random.h>
#include <ofxQuery/QueryResult.h>
#include "na/model/attract.h"
#include "na/support/generator.h"

namespace na {

/**
 * \class na::AttractQuery
 */
class AttractQuery : public na::Generator<Attract> {
  public:
    AttractQuery();

  protected:
    virtual bool                generate(std::vector<Attract>& out);

  private:
    bool                        query(std::vector<Attract>& out);
	  // Create the mFields map, throwing if anything goes wrong.
	  void						            buildFields(const QueryResult&);

    std::string                 mInput;
    QueryResult                 mResult;
    // Tmp for debugging
    Poco::Random                mRnd;

  	// This glorious construct is used to store a collection of functions that
  	// translate query results into story data.
  	std::vector<std::function<void(Attract&, const QueryResult::RowIterator&, const int index)>>
								                mFields;
};

} // namespace na

#endif // NA_QUERY_ATTRACTQUERY_H_