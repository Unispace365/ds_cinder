#include "ds/thread/work_client.h"

#include "ds/app/engine.h"
#include "ds/thread/work_manager.h"

namespace ds {

/**
 * \class ds::WorkClient
 */
WorkClient::WorkClient(Engine& e)
	: mManager(e.mWorkManager)
{
	mManager.addClient(*this);
}

WorkClient::~WorkClient()
{
	mManager.removeClient(*this);
}

void WorkClient::handleResult(std::unique_ptr<WorkRequest>&)
{
}

} // namespace ds
