#pragma once
#ifndef QUERY_INDUSTRY_QUERY_H_
#define QUERY_INDUSTRY_QUERY_H_

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "model/all_stories.h"

namespace fullstarter {

/**
 * \class fullstarter::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
public:
	StoryQuery();

	virtual void			run();

	AllStories				mOutput;

private:
	void					query(AllStories& output);
};

} // namespace fullstarter

#endif