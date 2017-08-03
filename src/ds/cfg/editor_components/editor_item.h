#pragma once
#ifndef DS_CFG_SETTINGS_EDITOR_EDITOR_ITEM
#define DS_CFG_SETTINGS_EDITOR_EDITOR_ITEM

#include <map>

#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/cfg/settings.h>

namespace ds{
namespace cfg{

/// A single item in the editor
class EditorItem : public ds::ui::Sprite {
public:
	EditorItem(ds::ui::SpriteEngine& e);

	void					setSetting(Settings::Setting* theSetting);

protected:
	Settings::Setting*		mTheSetting;
};

} // namespace ui
} // namespace ds
#endif // 