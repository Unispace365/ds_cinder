/**
* @file projects/essentials/src/ds/ui/util/text_model.h
*
* @brief ...
* @author Aubrey Francois
* @date 2020-04-07
*/
#include <ds/ui/sprite/sprite_engine.h>
#pragma once
#ifndef DS_CINDER_PROJECTS_ESSENTIALS_SRC_DS_UI_UTIL_TARGET_UTILS
#define DS_CINDER_PROJECTS_ESSENTIALS_SRC_DS_UI_UTIL_TARGET_UTILS


namespace ds {
	namespace ui {
		inline void setLayoutTarget(ds::ui::SpriteEngine& engine,std::string target) {
			ds::cfg::Settings::Setting& setting = engine.getEngineSettings().getSetting("xml_importer:target", 0);
			setting.mRawValue = target;

		};
		

	}
}

#endif