#ifndef _LAYOUT_BUILDER_APP_QUERY_INDUSTRY_QUERY_H_
#define _LAYOUT_BUILDER_APP_QUERY_INDUSTRY_QUERY_H_

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "model/all_stories.h"

namespace layout_builder {

/**
 * \class layout_builder::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
public:
	StoryQuery();

	virtual void			run();

	AllStories				mOutput;

private:
	void					query(AllStories& output);
};

} // !namespace layout_builder

#endif //!_LAYOUT_BUILDER_APP_QUERY_INDUSTRY_QUERY_H_