#include "stdafx.h"

#include "ds/app/app.h"
#include "ds/app/engine/engine_standalone.h"
#include "ds/content/content_wrangler.h"
#include "ds/debug/computer_info.h"

using namespace ci;
using namespace app;

namespace ds {

/**
 * \class EngineStandalone
 */
EngineStandalone::EngineStandalone(App& app, EngineSettings& settings, EngineData& ed, const RootList& roots)
  : Engine(app, settings, ed, roots, STANDALONE_MODE)
  , mContentWrangler(nullptr) {}

EngineStandalone::~EngineStandalone() {
	// It's important to clean up the sprites before the services go away
	clearAllSprites();

	// Important to do this here before the work manager is destructed.
	mData.clearServices();
}

void EngineStandalone::installSprite(const std::function<void(BlobRegistry&)>& asServer,
									 const std::function<void(BlobRegistry&)>& asClient) {
	// I don't have network communication so I don't need to handle blob.
}

void EngineStandalone::setup(App& app) {
	Engine::setup(app);

	if (!mContentWrangler) {
		mContentWrangler = new ContentWrangler(*this);
	}

	app.preServerSetup();
	app.setupServer();

	if (mContentWrangler) {
		mContentWrangler->initialize();
	}
}


void EngineStandalone::update() {
	mWorkManager.update();
	mComputerInfo->update();
	updateServer();
}

void EngineStandalone::draw() {
	drawClient();
}

void EngineStandalone::stopServices() {
	Engine::stopServices();
	mWorkManager.stopManager();
}

void EngineStandalone::handleMouseTouchBegin(const MouseEvent& e, int id) {
	mTouchManager.mouseTouchBegin(e, id);
}

void EngineStandalone::handleMouseTouchMoved(const MouseEvent& e, int id) {
	mTouchManager.mouseTouchMoved(e, id);
}

void EngineStandalone::handleMouseTouchEnded(const MouseEvent& e, int id) {
	mTouchManager.mouseTouchEnded(e, id);
}

} // namespace ds
