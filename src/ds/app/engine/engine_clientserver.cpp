#include "stdafx.h"

#include "ds/app/engine/engine_clientserver.h"

#include "ds/app/app.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 */
EngineClientServer::EngineClientServer(	ds::App& app, ds::EngineSettings& settings,
										ds::EngineData& ed, const ds::RootList& roots)
		: AbstractEngineServer(app, settings, ed, roots, CLIENTSERVER_MODE)
{
}

EngineClientServer::~EngineClientServer() {
}


void EngineClientServer::draw() {
	drawClient();
}

} // namespace ds
