#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR
#define DS_CFG_SETTINGS_EDITOR

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/cfg/settings.h>
#include <ds/ui/layout/layout_sprite.h>

namespace ds{
namespace cfg{
class EditorItem;
class EditView;

/// View for displaying and editing settings
/// Note that the ui is specified in c++ and not an external layout file
/// This is designed to work for any app without any sidecar files
/// So it's best not to use this as an example of best practices (unless you're also making a portable ui)
class SettingsEditor : public ds::ui::Sprite {
public:
	SettingsEditor(ds::ui::SpriteEngine& e);
	
	void						showSettings(Settings* theSettings);
	void						hideSettings();

	void						editProperty(EditorItem* ei);
private:
	EditorItem*					getNextItem(EditorItem* ei);
	EditorItem*					getPrevItem(EditorItem* ei);

	ds::ui::Text*				createSaveButton(ds::ui::LayoutSprite* theParent);
	void						setSaveTouch(ds::ui::Text* theButton, const std::string& saveName);

	Settings*					mCurrentSettings;

	ds::ui::LayoutSprite*		mPrimaryLayout;
	ds::ui::LayoutSprite*		mSettingsLayout;
	std::vector<EditorItem*>	mSettingItems;
	ds::ui::Text*				mTitle;
	ds::ui::Text*				mSaveApp;
	ds::ui::Text*				mSaveLocal;
	ds::ui::Text*				mSaveConfig;
	ds::ui::Text*				mSaveLocalConfig;
	EditView*					mEditView;
};

} // namespace ui
} // namespace ds
#endif // 