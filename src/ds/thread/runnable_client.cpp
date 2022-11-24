#include "stdafx.h"

#include "ds/thread/runnable_client.h"

#include "ds/thread/work_manager.h"
#include "ds/util/memory_ds.h"

namespace ds {

/**
 * \class RunnableClient
 */
RunnableClient::RunnableClient(ui::SpriteEngine& e, const std::function<void(std::unique_ptr<Poco::Runnable>&)>& h)
  : inherited(e)
  , mCache(this)
  , mResultHandler(h) {}

void RunnableClient::setResultHandler(const std::function<void(std::unique_ptr<Poco::Runnable>&)>& h) {
	mResultHandler = h;
}

bool RunnableClient::run(std::unique_ptr<Poco::Runnable>& payload) {
	if (!payload) return false;

	std::unique_ptr<Request> r(std::move(mCache.next()));
	if (!r) return false;

	r.get()->mPayload = std::move(payload);
	return mManager.sendRequest(ds::unique_dynamic_cast<WorkRequest, Request>(r));
}

void RunnableClient::handleResult(std::unique_ptr<WorkRequest>& wr) {
	std::unique_ptr<Request> r(ds::unique_dynamic_cast<Request, WorkRequest>(wr));
	if (!r) return;

	if (mResultHandler) mResultHandler(r.get()->mPayload);
	// Recycle the request.  It's up to clients to recycle the payload if they want to.
	mCache.push(r);
}

/**
 * \class Request
 */
RunnableClient::Request::Request(const void* clientId)
  : WorkRequest(clientId) {}

void RunnableClient::Request::run() {
	if (mPayload) mPayload->run();
}

} // namespace ds
