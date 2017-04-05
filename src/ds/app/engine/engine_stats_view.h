#pragma once
#ifndef DS_APP_ENGINE_ENGINESTATSVIEW_H_
#define DS_APP_ENGINE_ENGINESTATSVIEW_H_

// Hmmp... if I don't do this first, I get a ton of
// redefinition warnings.
#include "ds/app/engine/engine.h"

#include "ds/app/blob_registry.h"
#include "ds/app/event.h"
#include "ds/app/event_client.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/text.h"

namespace ds {
class Engine;

/**
 * \class ds::EngineStatsView
 * \brief Display basic stats.
 */
class EngineStatsView : public ds::ui::Sprite {
public:
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);
	EngineStatsView(ds::ui::SpriteEngine&);


	virtual void				onUpdateServer(const ds::UpdateParams&) override;
	virtual void				onUpdateClient(const ds::UpdateParams&) override;

	void						updateStats();
private:
	void						onAppEvent(const ds::Event&);
	ds::Engine&					mEngine;
	ds::EventClient				mEventClient;
	// UI
	ds::ui::Sprite*				mBackground;
	ds::ui::Text*				mText;

	// SETTINGS
	const ci::vec2				mLT;

	// EVENTS
public:
	class ToggleStatsRequest : public ds::RegisteredEvent<ToggleStatsRequest> {
	public:
		ToggleStatsRequest(){}
	};
};

} // namespace ds

#endif