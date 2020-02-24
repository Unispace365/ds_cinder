#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR_EDITOR_ITEM
#define DS_CFG_SETTINGS_EDITOR_EDITOR_ITEM

#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/cfg/settings.h>

namespace ds{
namespace cfg{

/// A single item in the SettingsEditor, to display the item and it's properties. Click on this to open it's edit view
class EditorItem : public ds::ui::LayoutSprite {
public:
	EditorItem(ds::ui::SpriteEngine& e);

	void					setSetting(Settings::Setting* theSetting);
	const std::string&		getSettingName();
	bool					getIsHeader(){ return mIsHeader; }
	bool					isDerived()
	{
		return mHasOriginalValue;
	}
protected:
	bool					mIsHeader;
	bool					mHasOriginalValue; //this setting is derived either from another or an expr (or both).
	std::string				mOriginalSettingName; // somehow gotta handle indexes?

	ds::ui::Text*			mSettingName;
	ds::ui::Text*			mSettingValue;
	ds::ui::Text*			mSettingComment;
};

} // namespace ui
} // namespace ds
#endif // 