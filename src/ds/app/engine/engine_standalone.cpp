#include "ds/app/engine/engine_standalone.h"

#include "ds/app/app.h"

#include <ds/debug/logger.h>
#include <ds/debug/computer_info.h>

using namespace ci;
using namespace ci::app;

namespace ds {

/**
 * \class ds::EngineStandalone
 */
EngineStandalone::EngineStandalone(	ds::App& app, const ds::cfg::Settings& settings,
									ds::EngineData& ed, const ds::RootList& roots)
		: inherited(app, settings, ed, roots)
		, mLoadImageService(*this, mIpFunctions)
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

	mRenderTextThread.start(true);

	app.setupServer();
}

void EngineStandalone::setupTuio(ds::App& a) {
	if (ds::ui::TouchMode::hasTuio(mTouchMode)) {
		ci::tuio::Client&		tuioClient = getTuioClient();
		tuioClient.registerTouches(&a);
		registerForTuioObjects(tuioClient);
		try{
			tuioClient.connect(mTuioPort);
		} catch (std::exception ex) {	
			DS_LOG_WARNING("Tuio client could not be started.");
		}
	}
}

void EngineStandalone::update() {
	mWorkManager.update();
	mRenderTextService.update();
	mComputerInfo->update();
	updateServer();
}

void EngineStandalone::draw() {
	drawClient();
}

void EngineStandalone::stopServices() {
	inherited::stopServices();
	mWorkManager.stopManager();
}

void EngineStandalone::handleMouseTouchBegin(const ci::app::MouseEvent& e, int id){
	mTouchManager.mouseTouchBegin(e, id);
}

void EngineStandalone::handleMouseTouchMoved(const ci::app::MouseEvent& e, int id){
	mTouchManager.mouseTouchMoved(e, id);
}

void EngineStandalone::handleMouseTouchEnded(const ci::app::MouseEvent& e, int id){
	mTouchManager.mouseTouchEnded(e, id);
}

} // namespace ds
