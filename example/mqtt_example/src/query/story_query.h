#ifndef _MQTT_EXAMPLE_APP_QUERY_INDUSTRY_QUERY_H_
#define _MQTT_EXAMPLE_APP_QUERY_INDUSTRY_QUERY_H_

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "model/all_data.h"

namespace example {

/**
 * \class example::StoryQuery
 */
class StoryQuery : public Poco::Runnable {
public:
	StoryQuery();

	virtual void			run();

	AllData					mOutput;

private:
	void					query(AllData& output);
};

} // !namespace example

#endif //!_MQTT_EXAMPLE_APP_QUERY_INDUSTRY_QUERY_H_