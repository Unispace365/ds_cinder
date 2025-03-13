#include "stdafx.h"

#include "ds/app/app.h"
#include "ds/app/engine/engine_clientserver.h"

namespace ds {

/**
 * \class EngineClientServer
 */
EngineClientServer::EngineClientServer(App& app, EngineSettings& settings, EngineData& ed, const RootList& roots)
  : AbstractEngineServer(app, settings, ed, roots, CLIENTSERVER_MODE) {}

EngineClientServer::~EngineClientServer() {}


void EngineClientServer::draw() {
	drawClient();
}

} // namespace ds
