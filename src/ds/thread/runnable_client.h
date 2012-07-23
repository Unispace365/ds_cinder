#pragma once
#ifndef DS_THREAD_RUNNABLECLIENT_H_
#define DS_THREAD_RUNNABLECLIENT_H_

#include <functional>
#include "ds/thread/work_client.h"
#include "ds/thread/work_request_list.h"

namespace ds {

/**
 * \class ds::RunnableClient
 * \brief Handle simple runnable operations.  Clients are responsible for recycling
 * any runnables supplied to run() (and received back fromt eh result handler), if
 * they want to.  Otherwise, the object is memory managed.
 */
class RunnableClient : public WorkClient {
public:
	RunnableClient(ui::SpriteEngine&, const std::function<void(std::unique_ptr<Poco::Runnable>&)>& = nullptr);
	
	void						setResultHandler(const std::function<void(std::unique_ptr<Poco::Runnable>&)>&);

	bool						run(std::unique_ptr<Poco::Runnable>&);

protected:
	virtual void				handleResult(std::unique_ptr<WorkRequest>&);

private:
	typedef WorkClient			inherited;

	class Request : public WorkRequest {
	public:
		Request(const void* clientId);

		std::unique_ptr<Poco::Runnable>
								mPayload;

		void					run();
	};
	WorkRequestList<Request>	mCache;

	std::function<void(std::unique_ptr<Poco::Runnable>&)>
								mResultHandler;
};

} // namespace ds

#endif // DS_THREAD_RUNNABLECLIENT_H_
