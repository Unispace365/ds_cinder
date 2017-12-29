#ifndef _TRIANGLE_MAN_APP_QUERY_INDUSTRY_QUERY_H_
#define _TRIANGLE_MAN_APP_QUERY_INDUSTRY_QUERY_H_

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "model/all_stories.h"

namespace nwm {

/**
 * \class nwm::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
public:
	StoryQuery();

	virtual void			run();

	AllStories				mOutput;

private:
	void					query(AllStories& output);
};

} // !namespace nwm

#endif //!_TRIANGLE_MAN_APP_QUERY_INDUSTRY_QUERY_H_

