#pragma once
#ifndef DS_APP_ENGINE_ENGINECLIENTSERVER_H_
#define DS_APP_ENGINE_ENGINECLIENTSERVER_H_

#include "ds/app/engine/engine_server.h"

namespace ds {

/**
 * \class EngineClientServer
 * The ClientServer engine contains all behaviour found in both the client
 * and server, and no communication pipe replicating sprite changes.
 */
class EngineClientServer : public AbstractEngineServer {
  public:
	EngineClientServer(App&, EngineSettings&, EngineData&, const RootList&);
	~EngineClientServer() override;


	void draw() override;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINECLIENTSERVER_H_
