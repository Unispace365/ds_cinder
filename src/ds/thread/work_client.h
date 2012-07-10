#pragma once
#ifndef DS_THREAD_WORKCLIENT_H_
#define DS_THREAD_WORKCLIENT_H_

#include <vector>
#include "ds/thread/work_request.h"

namespace ds {
class Engine;
class WorkManager;

/**
 * \class ds::WorkClient
 * \brief Abstract superclass for anything that can supply requests to the work manager.
 */
class WorkClient {
public:
	WorkClient(Engine&);
	virtual ~WorkClient();

protected:
	friend class WorkManager;

	// Subclasses should take ownership if they want to recycle the request.
	virtual void			handleResult(std::unique_ptr<WorkRequest>&);

protected:
	WorkManager&			mManager;
};

} // namespace ds

#endif // DS_THREAD_WORKCLIENT_H_
