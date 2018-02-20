#pragma  once
#ifndef _FULLSTARTER_APP_QUERY_STORY_QUERY_H_
#define _FULLSTARTER_APP_QUERY_STORY_QUERY_H_

#include <Poco/Runnable.h>

#include "model/all_data.h"

namespace fullstarter {

class StoryQuery : public Poco::Runnable {
public:
	StoryQuery();

	virtual void			run();

	AllData					mOutput;

private:
	void					query(AllData& output);
};

} // !namespace fullstarter

#endif //!_FULLSTARTER_APP_QUERY_INDUSTRY_QUERY_H_