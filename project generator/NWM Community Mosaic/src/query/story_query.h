#pragma  once
#ifndef _NWM COMMUNITY MOSAIC_APP_QUERY_STORY_QUERY_H_
#define _NWM COMMUNITY MOSAIC_APP_QUERY_STORY_QUERY_H_

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "model/all_data.h"

namespace nwm {

/**
 * \class nwm::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
public:
	StoryQuery();

	virtual void			run();

	AllData					mOutput;

private:
	void					query(AllData& output);
};

} // !namespace nwm

#endif //!_NWM COMMUNITY MOSAIC_APP_QUERY_INDUSTRY_QUERY_H_