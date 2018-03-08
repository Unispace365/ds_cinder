#pragma once
#ifndef _GENERIC_DATA_MODEL_APP_UI_DATA_LAYOUT_H_
#define _GENERIC_DATA_MODEL_APP_UI_DATA_LAYOUT_H_

#include <ds/ui/layout/smart_layout.h>
#include "model/data_model.h"

namespace ds {
namespace ui {

/**
* \class ds::ui::DataLayout
*/
class DataLayout : public ds::ui::SmartLayout {
public:
	DataLayout(ds::ui::SpriteEngine&, const std::string& xmlLayoutFile, ds::model::DataModelRef theData = ds::model::DataModelRef(),
			   const std::string xmlFileLocation = "%APP%/data/layouts/");

	void setData(ds::model::DataModelRef& theData);
	
};

} // namespace ui
} // namespace ds

#endif
