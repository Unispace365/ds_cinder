#ifndef _MEDIAVIEWER_APP_QUERY_INDUSTRY_QUERY_H_
#define _MEDIAVIEWER_APP_QUERY_INDUSTRY_QUERY_H_

#include <functional>
#include <Poco/Runnable.h>
#include <ds/query/query_result.h>

#include "model/all_data.h"

namespace mv {

/**
 * \class mv::MediaQuery
 */
class MediaQuery : public Poco::Runnable {
public:
	MediaQuery();

	virtual void			run();

	AllData					mOutput;

private:
	void					query(AllData& output);
};

} // !namespace mv

#endif //!_MEDIAVIEWER_APP_QUERY_INDUSTRY_QUERY_H_

