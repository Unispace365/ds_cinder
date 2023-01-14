#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR
#define DS_CFG_SETTINGS_EDITOR

#include <ds/cfg/settings.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds::cfg {
class EditorItem;
class EditView;

/// View for displaying and editing settings
/// Note that the ui is specified in c++ and not an external layout file
/// This is designed to work for any app without any sidecar files
/// So it's best not to use this as an example of best practices (unless you're also making a portable ui)
class SettingsEditor : public ds::ui::Sprite {
  public:
	SettingsEditor(ds::ui::SpriteEngine& e);

	void showSettings(const std::string settingsName);
	void hideSettings();

	virtual void drawPostLocalClient() override;

  private:
	void drawSettings();
	void drawSettingFile(ds::cfg::Settings& eng);
	void drawSingleSetting(ds::cfg::Settings::Setting& setting, ds::cfg::Settings& allSettings);

	void drawSaveButtons(ds::cfg::Settings& toSave);
	void saveChange(const std::string& path, ds::cfg::Settings& toSave);

	Settings* mCurrentSettings;

	bool mSettingsOpen = false;

	// EditView*				 mEditView;
};

} // namespace ds::cfg
#endif //
