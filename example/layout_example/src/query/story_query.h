#ifndef _LAYOUT_EXAMPLE_APP_QUERY_INDUSTRY_QUERY_H_
#define _LAYOUT_EXAMPLE_APP_QUERY_INDUSTRY_QUERY_H_

#include <Poco/Runnable.h>
#include <ds/query/query_result.h>
#include <functional>

#include "model/all_stories.h"

namespace example {

/**
 * \class example::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
  public:
	StoryQuery();

	virtual void run();

	AllStories mOutput;

  private:
	void query(AllStories& output);
};

} // namespace example

#endif //!_LAYOUT_EXAMPLE_APP_QUERY_INDUSTRY_QUERY_H_
