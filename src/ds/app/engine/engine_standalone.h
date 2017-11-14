#pragma once
#ifndef DS_APP_ENGINE_ENGINESTANDALONE_H_
#define DS_APP_ENGINE_ENGINESTANDALONE_H_

#include "ds/app/engine/engine.h"
#include "ds/thread/work_manager.h"
#include "ds/ui/service/load_image_service.h"

namespace ds {

/**
 * \class ds::EngineStandalone
 * The Standalone engine contains all behaviour found in both the client
 * and server, and no communication pipe replicating sprite changes.
 */
class EngineStandalone : public Engine {
public:
	EngineStandalone(ds::App&, ds::EngineSettings&, ds::EngineData&, const ds::RootList&);
	~EngineStandalone();

	virtual ds::WorkManager&		getWorkManager()		{ return mWorkManager; }
	virtual ui::LoadImageService&	getLoadImageService()	{ return mLoadImageService; }

	virtual void					installSprite(const std::function<void(ds::BlobRegistry&)>& asServer,
													const std::function<void(ds::BlobRegistry&)>& asClient);

	virtual void					setup(ds::App&);
	virtual void					update();
	virtual void					draw();

	virtual void					stopServices();
	virtual int						getMode() const { return STANDALONE_MODE; }

	virtual int						getBytesRecieved(){ return 0; }
	virtual int						getBytesSent(){ return 0; }

private:
	typedef Engine inherited;

	virtual void					handleMouseTouchBegin(const ci::app::MouseEvent&, int id);
	virtual void					handleMouseTouchMoved(const ci::app::MouseEvent&, int id);
	virtual void					handleMouseTouchEnded(const ci::app::MouseEvent&, int id);

	WorkManager						mWorkManager;
	ui::LoadImageService			mLoadImageService;
};

} // namespace ds

#endif
