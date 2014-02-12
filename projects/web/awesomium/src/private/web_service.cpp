#include "private/web_service.h"

#include <ds/app/engine/engine.h>
#include "CinderAwesomium.h"

namespace ds {
namespace web {

/**
 * \class ds::web::Service
 */
Service::Service(ds::Engine& e)
	: ds::AutoUpdate(e)
	, mWebCorePtr(nullptr)
{
}

Service::~Service()
{
	if (mWebCorePtr) {
		Awesomium::WebCore::Shutdown();
	}
}

void Service::start()
{
	Awesomium::WebConfig cnf;
	cnf.log_level = Awesomium::kLogLevel_Verbose;

	// initialize the Awesomium web engine
	mWebCorePtr = Awesomium::WebCore::Initialize(cnf);

	std::cout << mWebCorePtr->version_string() << std::endl;
}

Awesomium::WebCore* Service::getWebCore() const
{
	return mWebCorePtr;
}

void Service::update(const ds::UpdateParams&)
{
	if (mWebCorePtr) {
		mWebCorePtr->Update();
	}
}

} // namespace web
} // namespace ds