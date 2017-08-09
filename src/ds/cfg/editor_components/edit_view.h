#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR_EDITOR_VIEW
#define DS_CFG_SETTINGS_EDITOR_EDITOR_VIEW

#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/cfg/settings.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/soft_keyboard/entry_field.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>

namespace ds{
namespace cfg{

/// The view to edit a single item
class EditView : public ds::ui::LayoutSprite {
public:
	EditView(ds::ui::SpriteEngine& e);

	void					setSetting(Settings::Setting* theSetting, const std::string& parentSettingsName);
	void					updateValue(const std::wstring& theValue);
	Settings::Setting*		getSetting(){ return mTheSetting; }

	void					setSettingUpdatedCallback(std::function<void(Settings::Setting*)> func){ mSettingUpdatedCalback = func; }
	void					setRequestNextSettingCallback(std::function<void(const bool isNext)> func){ mNextSettingCallback = func; }

	void					stopEditing();

protected:
	Settings::Setting*		mTheSetting;
	std::string				mParentSettingsName;

	ds::ui::Text*			mSettingName;
	ds::ui::Text*			mSettingValue;
	ds::ui::Text*			mSettingComment;
	ds::ui::Text*			mSettingDefault;
	ds::ui::Text*			mSettingMin;
	ds::ui::Text*			mSettingMax;
	ds::ui::Text*			mSettingSource;
	ds::ui::Text*			mApplyButton;
	ds::ui::EntryField*		mEntryEditor;
	ds::ui::SoftKeyboard*	mKeyboard;

	std::function<void(Settings::Setting*)> mSettingUpdatedCalback;
	std::function<void(const bool)>	mNextSettingCallback;

	ds::ui::Text*			addTextSprite(const std::string& fontName, const float fontSize, const float opacity, const bool clickSetValue);
};

} // namespace ui
} // namespace ds
#endif // 