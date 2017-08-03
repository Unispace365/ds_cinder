#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR
#define DS_CFG_SETTINGS_EDITOR

#include <map>

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/cfg/settings.h>

namespace ds{
namespace cfg{
class EditorItem;

/// View for displaying and editing settings
class SettingsEditor : public ds::ui::Sprite {
public:
	SettingsEditor(ds::ui::SpriteEngine& e);
	
	void						showSettings(Settings* theSettings);

private:
	Settings*					mCurrentSettings;

	std::vector<EditorItem*>	mSettingItems;
};

} // namespace ui
} // namespace ds
#endif // 