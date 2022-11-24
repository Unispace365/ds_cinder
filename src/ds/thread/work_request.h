#pragma once
#ifndef DS_THREAD_WORKREQUEST_H_
#define DS_THREAD_WORKREQUEST_H_

#include <Poco/Runnable.h>
#include <Poco/Timestamp.h>

namespace ds {

/**
 * \class WorkRequest
 * \brief Abstract class for anything that can be sent into the work manager.
 */
class WorkRequest : public Poco::Runnable {
  public:
	WorkRequest(const void* clientId);
	virtual ~WorkRequest();

  protected:
	friend class WorkManager;

	const void*		mClientId;
	Poco::Timestamp mRequestTime;

  private:
	WorkRequest();
};

} // namespace ds

#endif // DS_THREAD_WORKREQUEST_H_
