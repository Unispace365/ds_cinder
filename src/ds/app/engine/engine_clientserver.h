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
	EngineClientServer(ds::App&, const ds::cfg::Settings&, ds::EngineData&, const ds::RootList&);
	~EngineClientServer();

	virtual ui::LoadImageService&	getLoadImageService()	{ return mLoadImageService; }
	virtual ui::RenderTextService&	getRenderTextService()	{ return mRenderTextService; }

	virtual void					setup(ds::App&);
	virtual void					draw();

	virtual int						getMode() const { return CLIENTSERVER_MODE; }

private:
	typedef AbstractEngineServer inherited;
	GlThread						mLoadImageThread;
	ui::LoadImageService			mLoadImageService;
	GlThread						mRenderTextThread;
	ui::RenderTextService			mRenderTextService;
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINECLIENTSERVER_H_