#include "stdafx.h"

#include "ds/thread/work_request.h"

namespace ds {

/**
 * \class WorkRequest
 */
WorkRequest::WorkRequest(const void* clientId)
	: mClientId(clientId)
{
}

WorkRequest::~WorkRequest()
{
}

} // namespace ds
