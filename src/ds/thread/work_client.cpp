#include "stdafx.h"

#include "ds/thread/work_client.h"

#include "ds/thread/work_manager.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {

/**
 * \class WorkClient
 */
WorkClient::WorkClient(ui::SpriteEngine& e)
	: mManager(e.getWorkManager())
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
