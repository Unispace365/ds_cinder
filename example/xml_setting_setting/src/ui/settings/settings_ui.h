#pragma once
#ifndef DS_UI_SETTINGS_SETTINGS_UI_H_
#define DS_UI_SETTINGS_SETTINGS_UI_H_

#include <ds/ui/sprite/sprite.h>
#include <cinder/params/Params.h>
#include <ds/cfg/settings.h>

namespace setter {
class Globals;
/**
* \class exxon::SearchView
*		A single instance of the search view, belongs to a single placemat
*/
class SettingsUi final : public ds::ui::Sprite  {
public:

	SettingsUi(Globals& g);
	virtual ~SettingsUi();

	void						toggleVisibility();
private:
	Globals&					mGlobals;
	virtual void				drawLocalClient();

	void						registerSettings(const ds::cfg::Settings* settings);

	ci::params::InterfaceGlRef	mParams;
	std::string					mTest;
	bool						mDrawParams;

};

} // namespace setter

#endif


