#ifndef _MEDIAVIEWER_APP_GLOBALS_
#define _MEDIAVIEWER_APP_GLOBALS_

#include <ds/app/event_notifier.h>
#include <ds/cfg/cfg_text.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds { namespace ui {
	class SpriteEngine;
}} // namespace ds::ui

namespace mv {
class ViewerController;
/**
 * \class mv::Globals
 * \brief Global data for the app.
 */
class Globals {
  public:
	Globals(ds::ui::SpriteEngine&);

	ds::ui::SpriteEngine& mEngine;
	ds::EventNotifier	  mNotifier;
	float				  mVolume;
	void				  initialize();

	const float getAnimDur();

	// Shortcuts
	const ds::cfg::Text& getText(const std::string& name) const;
	ds::cfg::Settings&	 getAppSettings() const;
	ds::cfg::Settings&	 getSettings(const std::string& name) const;

  private:
	float mAnimDuration;
	int	  mRequestId;
};

} // namespace mv

#endif // !_MEDIAVIEWER_APP_GLOBALS_
