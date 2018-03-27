#pragma once
#ifndef DS_APP_ENGINE_ENGINECLIENTSERVER_H_
#define DS_APP_ENGINE_ENGINECLIENTSERVER_H_

#include "ds/app/engine/engine_server.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 * The ClientServer engine contains all behaviour found in both the client
 * and server, and no communication pipe replicating sprite changes.
 */
class EngineClientServer : public AbstractEngineServer {
public:
	EngineClientServer(ds::App&, ds::EngineSettings&, ds::EngineData&, const ds::RootList&);
	~EngineClientServer();


	virtual void					draw();

};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINECLIENTSERVER_H_
