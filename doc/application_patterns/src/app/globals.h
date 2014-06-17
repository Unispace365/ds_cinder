#pragma once
#ifndef AP_APP_GLOBALS
#define AP_APP_GLOBALS

namespace na {
class App;
class EventClient;
class NodeTree;

/**
 * \class na::Globals
 * Provide access to all app data.
 */
class Globals {
public:
	Globals(ds::ui::SpriteEngine&, const NodeTree&);

	ds::ui::SpriteEngine&			mEngine;

	// Model
	const na::NodeTree&				mNodeTree;

	// Shortcuts
	const ds::cfg::Settings&		getSettingsLayout() const;
	const ds::cfg::Settings&		getSettings(const std::string& name) const;
	const ds::cfg::Text&			getText(const std::string& name) const;
	const ds::cfg::NinePatch&		getNinePatch(const std::string& name) const;
};

} // namespace ju

#endif // AP_APP_GLOBALS
