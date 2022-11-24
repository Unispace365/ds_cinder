#pragma once
#ifndef _FINGER_DRAWING_APP_QUERY_STORY_QUERY_H_
#define _FINGER_DRAWING_APP_QUERY_STORY_QUERY_H_

#include <Poco/Runnable.h>
#include <ds/query/query_result.h>
#include <functional>

#include "model/all_data.h"

namespace example {

/**
 * \class example::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
  public:
	StoryQuery();

	virtual void run();

	AllData mOutput;

  private:
	void query(AllData& output);
};

} // namespace example

#endif //!_FINGER_DRAWING_APP_QUERY_INDUSTRY_QUERY_H_