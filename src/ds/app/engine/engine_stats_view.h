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
#include "ds/debug/apphost_stats_view.h"

namespace ds {
class Engine;

/**
 * \class EngineStatsView
 * \brief Display basic stats.
 */
class EngineStatsView : public ds::ui::Sprite {
public:
	/// A request to show or hide the stats view (# of sprites, memory use, fps)
	class ToggleStatsRequest : public ds::RegisteredEvent<ToggleStatsRequest> {};
	/// Show the available keys and the stats view if hidden
	class ToggleHelpRequest : public ds::RegisteredEvent<ToggleHelpRequest> {};

	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);
	EngineStatsView(ds::ui::SpriteEngine&);


	virtual void				onUpdateServer(const ds::UpdateParams&) override;
	virtual void				onUpdateClient(const ds::UpdateParams&) override;

	void						updateStats();
private:
	ds::Engine&					mEngine;
	ds::EventClient				mEventClient;
	/// UI
	ds::ui::Sprite*				mBackground;
	ds::ui::Text*				mText;
	ds::ui::AppHostStatsView*	mAppHostStats;

	bool						mShowingHelp;
};

} // namespace ds

#endif
