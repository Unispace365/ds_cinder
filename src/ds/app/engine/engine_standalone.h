#pragma once
#ifndef DS_APP_ENGINE_ENGINESTANDALONE_H_
#define DS_APP_ENGINE_ENGINESTANDALONE_H_

#include "ds/app/engine/engine.h"

namespace ds {

class ContentWrangler;

/**
 * \class EngineStandalone
 * The Standalone engine contains all behaviour found in both the client
 * and server, and no communication pipe replicating sprite changes.
 */
class EngineStandalone : public Engine {
  public:
	EngineStandalone(App&, EngineSettings&, EngineData&, const RootList&);
	~EngineStandalone() override;

	void installSprite(const std::function<void(BlobRegistry&)>& asServer,
					   const std::function<void(BlobRegistry&)>& asClient) override;

	void setup(App&) override;
	void update() override;
	void draw() override;

	void stopServices() override;
	// virtual int	 getMode() const { return STANDALONE_MODE; }

	int getBytesReceived() override { return 0; }
	int getBytesSent() override { return 0; }

  private:
	void handleMouseTouchBegin(const ci::app::MouseEvent&, int id) override;
	void handleMouseTouchMoved(const ci::app::MouseEvent&, int id) override;
	void handleMouseTouchEnded(const ci::app::MouseEvent&, int id) override;

	ContentWrangler* mContentWrangler;
};

} // namespace ds

#endif
