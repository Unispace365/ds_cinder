#include "stdafx.h"

#include "data_layout.h"

namespace ds {
namespace ui {

DataLayout::DataLayout(ds::ui::SpriteEngine& eng, const std::string& xmlLayoutFile, ds::model::DataModelRef theData, const std::string xmlFileLocation /*= "%APP%/data/layouts/"*/) 
	: SmartLayout(eng, xmlLayoutFile, xmlFileLocation)
{
	setData(theData);

}

void DataLayout::setData(ds::model::DataModelRef& theData) {
	for (auto it : theData.getProperties()){
		if(hasSprite(it.first)) {
			if(it.second.getResource().empty()) {
				setSpriteText(it.first, it.second.getString());
			} else {
				setSpriteImage(it.first, it.second.getResource());
			}
		}		
	}

	runLayout();
}

}
}