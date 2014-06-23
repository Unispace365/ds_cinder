#include "ds/app/engine/engine_clientserver.h"

#include "ds/app/app.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 */
EngineClientServer::EngineClientServer(	ds::App& app, const ds::cfg::Settings& settings,
										ds::EngineData& ed, const ds::RootList& roots)
		: inherited(app, settings, ed, roots)
		, mLoadImageService(mLoadImageThread, mIpFunctions)
		, mRenderTextService(mRenderTextThread) {
}

EngineClientServer::~EngineClientServer() {
}

void EngineClientServer::setup(ds::App& app) {
	inherited::setup(app);

	mLoadImageThread.start(true);
	mRenderTextThread.start(true);
}

void EngineClientServer::draw() {
	drawClient();
}

} // namespace ds
