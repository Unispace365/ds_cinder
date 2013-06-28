#include "private/pdf_service.h"

namespace ds {
namespace pdf {

/**
 * \class ds::pdf::PdfService
 */
Service::Service()
{
}

Service::~Service()
{
	mThread.waitForNoInput();
}

void Service::start()
{
	mThread.start(false);
}

} // namespace pdf
} // namespace ds