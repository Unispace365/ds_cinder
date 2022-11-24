#ifndef _PANORAMICVIDEO_APP_QUERY_INDUSTRY_QUERY_H_
#define _PANORAMICVIDEO_APP_QUERY_INDUSTRY_QUERY_H_

#include <Poco/Runnable.h>
#include <ds/query/query_result.h>
#include <functional>

#include "model/all_data.h"

namespace panoramic {

/**
 * \class panoramic::StoryQuery
 */
class DirectoryLoader : public Poco::Runnable {
  public:
	DirectoryLoader();

	virtual void run();

	AllData mOutput;

  private:
	void query(AllData& output);
};

} // namespace panoramic

#endif //!_PANORAMICVIDEO_APP_QUERY_INDUSTRY_QUERY_H_
