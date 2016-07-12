#ifndef _CEFDEVELOP_APP_QUERY_INDUSTRY_QUERY_H_
#define _CEFDEVELOP_APP_QUERY_INDUSTRY_QUERY_H_

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "model/all_data.h"

namespace cef {

/**
 * \class cef::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
public:
	StoryQuery();

	virtual void			run();

	AllData					mOutput;

private:
	void					query(AllData& output);
};

} // !namespace cef

#endif //!_CEFDEVELOP_APP_QUERY_INDUSTRY_QUERY_H_