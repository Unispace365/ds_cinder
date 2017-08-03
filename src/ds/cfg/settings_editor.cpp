#include "stdafx.h"

#include "settings_editor.h"

#include <ds/math/math_defs.h>
#include "ds/app/blob_reader.h"
#include "ds/app/blob_registry.h"
#include "ds/cfg/editor_components/editor_item.h"

namespace ds{
namespace cfg{

SettingsEditor::SettingsEditor(ds::ui::SpriteEngine& e)
	: ds::ui::Sprite(e)
	, mCurrentSettings(nullptr)
{
}


void SettingsEditor::showSettings(Settings* theSettings){
	for (auto it : mSettingItems){
		it->release();
	}

	mSettingItems.clear();

}

}
}
