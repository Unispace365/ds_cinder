#include "ds/app/engine/engine_clientserver.h"

#include "ds/app/app.h"

namespace ds {

/**
 * \class ds::EngineClientServer
 */
EngineClientServer::EngineClientServer(	ds::App& app, const ds::cfg::Settings& settings,
										ds::EngineData& ed, const std::vector<int>* roots)
	: inherited(app, settings, ed, roots)
	, mLoadImageService(mLoadImageThread)
	, mRenderTextService(mRenderTextThread)
{
}

EngineClientServer::~EngineClientServer()
{
	// It's important to clean up the sprites before the services go away
	clearAllSprites();

	// Important to do this here before the work manager is destructed.
	mData.clearServices();
}

void EngineClientServer::installSprite(const std::function<void(ds::BlobRegistry&)>& asServer,
                                       const std::function<void(ds::BlobRegistry&)>& asClient)
{
  // I don't have network communication so I don't need to handle blob.
}

void EngineClientServer::setup(ds::App& app)
{
  inherited::setup(app);

  mLoadImageThread.start(true);
  mRenderTextThread.start(true);

  app.setupServer();
}

void EngineClientServer::setupTuio(ds::App& a) {
	tuio::Client&		tuioClient = getTuioClient();
	tuioClient.registerTouches(&a);
	registerForTuioObjects(tuioClient);
	tuioClient.connect(mTuioPort);
}

void EngineClientServer::update()
{
  mWorkManager.update();
  mRenderTextService.update();
  updateServer();
}

void EngineClientServer::draw()
{
  drawClient();
}

void EngineClientServer::stopServices()
{
  inherited::stopServices();
  mWorkManager.stopManager();
}

} // namespace ds
