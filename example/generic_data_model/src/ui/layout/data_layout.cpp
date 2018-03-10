#include "stdafx.h"

#include "data_layout.h"

#include <ds/util/string_util.h>

namespace ds {
namespace ui {

DataLayout::DataLayout(ds::ui::SpriteEngine& eng, const std::string& xmlLayoutFile, ds::model::DataModelRef theData, const std::string xmlFileLocation /*= "%APP%/data/layouts/"*/) 
	: SmartLayout(eng, xmlLayoutFile, xmlFileLocation)
{
	setData(theData);

}

void DataLayout::setData(ds::model::DataModelRef& theData) {
	for(auto it : mSpriteMap) {
		auto theModel = it.second->getUserData().getString("model");
		if(!theModel.empty()) {
			auto models = ds::split(theModel, "; ", true);
			for (auto mit : models){
				auto keyVals = ds::split(mit, ":", true);
				if(keyVals.size() == 2) {
					auto childProps = ds::split(keyVals[1], "->", true);
					if(childProps.size() == 2) {
						auto sprPropToSet = keyVals[0];
						auto theChild = childProps[0];
						auto theProp = childProps[1];
						std::string actualValue = "";

						if(sprPropToSet == "resource") {
							setSpriteImage(it.first, theData.getProperty(theProp).getResource());

						} else {
							if(theChild == "this") {
								actualValue = theData.getPropertyString(theProp);
							} else {
								actualValue = theData.getChildByName(theChild).getPropertyString(theProp);
							}

							if(!actualValue.empty()) {
								ds::ui::XmlImporter::setSpriteProperty(*it.second, sprPropToSet, actualValue);
							}
						}

					} else {
						DS_LOG_WARNING("DataLayout::setData() Invalid syntax for child / property mapping");
					}
				} else {
					DS_LOG_WARNING("DataLayout::setData() Invalid syntax for prop / model mapping");
				}
			}
		}
	}

	/*
	for (auto it : theData.getProperties()){
		if(hasSprite(it.first)) {
			if(it.second.getResource().empty()) {
				setSpriteText(it.first, it.second.getString());
			} else {
				setSpriteImage(it.first, it.second.getResource());
			}
		}		
	}
	*/

	runLayout();
}

}
}