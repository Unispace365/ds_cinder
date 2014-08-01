#pragma once
#ifndef DS_APP_ENGINE_ENGINESTATSVIEW_H_
#define DS_APP_ENGINE_ENGINESTATSVIEW_H_

// Hmmp... if I don't do this first, I get a ton of
// redefinition warnings.
#include "ds/app/engine/engine.h"

#include <cinder/Font.h>
#include <cinder/gl/TextureFont.h>
#include "ds/app/event.h"
#include "ds/app/event_client.h"
#include "ds/ui/sprite/sprite.h"

namespace ds {
class Engine;

/**
 * \class ds::EngineStatsView
 * \brief Display basic stats.
 */
class EngineStatsView : public ds::ui::Sprite {
public:
	EngineStatsView(ds::Engine&);

	virtual void				updateServer(const ds::UpdateParams&);
	virtual void				drawLocalClient();

private:
	float						drawLine(const std::string&, const float y);
	void						onAppEvent(const ds::Event&);
	void						makeTextureFont();

	typedef ds::ui::Sprite		inherited;
	ds::Engine&					mEngine;
	ds::EventClient				mEventClient;
	// UI
	ci::Font					mFont;
	ci::gl::TextureFontRef		mTextureFont;
	// SETTINGS
	const float					mFontSize;
	const ci::Vec2f				mBorder;

	// EVENTS
public:
	class Toggle : public ds::Event {
	public:
		static int WHAT();
		Toggle();
	};
};

} // namespace ds

#endif