#include "ds/app/engine/engine_standalone.h"

#include "ds/app/app.h"

namespace ds {

/**
 * \class ds::EngineStandalone
 */
EngineStandalone::EngineStandalone(	ds::App& app, const ds::cfg::Settings& settings,
									ds::EngineData& ed, const ds::RootList& roots)
		: inherited(app, settings, ed, roots)
		, mLoadImageService(mLoadImageThread, mIpFunctions)
		, mRenderTextService(mRenderTextThread) {
}

EngineStandalone::~EngineStandalone() {
	// It's important to clean up the sprites before the services go away
	clearAllSprites();

	// Important to do this here before the work manager is destructed.
	mData.clearServices();
}

void EngineStandalone::installSprite(	const std::function<void(ds::BlobRegistry&)>& asServer,
                                       const std::function<void(ds::BlobRegistry&)>& asClient) {
	// I don't have network communication so I don't need to handle blob.
}

void EngineStandalone::setup(ds::App& app) {
	inherited::setup(app);

	mLoadImageThread.start(true);
	mRenderTextThread.start(true);

	app.setupServer();
}

void EngineStandalone::setupTuio(ds::App& a) {
	if (ds::ui::TouchMode::hasTuio(mTouchMode)) {
		tuio::Client&		tuioClient = getTuioClient();
		tuioClient.registerTouches(&a);
		registerForTuioObjects(tuioClient);
		tuioClient.connect(mTuioPort);
	}
}

void EngineStandalone::update() {
	mWorkManager.update();
	mRenderTextService.update();
	updateServer();
}

void EngineStandalone::draw() {
	drawClient();
}

void EngineStandalone::stopServices() {
	inherited::stopServices();
	mWorkManager.stopManager();
}

} // namespace ds
