#ifndef _PERSPECTIVEPICKING_APP_QUERY_INDUSTRY_QUERY_H_
#define _PERSPECTIVEPICKING_APP_QUERY_INDUSTRY_QUERY_H_

#include <Poco/Runnable.h>
#include <ds/query/query_result.h>
#include <functional>

#include "model/all_stories.h"

namespace perspective_picking {

/**
 * \class perspective_picking::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
  public:
	StoryQuery();

	virtual void run();

	AllStories mOutput;

  private:
	void query(AllStories& output);
};

} // namespace perspective_picking

#endif //!_PERSPECTIVEPICKING_APP_QUERY_INDUSTRY_QUERY_H_
