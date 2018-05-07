#include "stdafx.h"

#include "smart_layout.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>
#include <ds/util/string_util.h>


namespace ds {
namespace ui {

SmartLayout::SmartLayout(ds::ui::SpriteEngine& engine, const std::string& xmlLayoutFile,
						 const std::string xmlFileLocation, const bool loadImmediately)
  : ds::ui::LayoutSprite(engine)
  , mLayoutFile(xmlFileLocation + xmlLayoutFile)
  , mNeedsLayout(false)
  , mInitialized(false)
  , mEventClient(engine) {
	if (loadImmediately) {
		initialize();
	}
}

SmartLayout::SmartLayout(ds::ui::SpriteEngine& engine)
	: ds::ui::LayoutSprite(engine)
	, mLayoutFile("")
	, mInitialized(false)
	, mNeedsLayout(false)
	, mEventClient(engine)
{

}

void SmartLayout::setLayoutFile(const std::string& xmlLayoutFile, const std::string xmlFileLocation,
								const bool loadImmediately) {
	mInitialized = false;
	mLayoutFile  = xmlFileLocation + xmlLayoutFile;

	if (loadImmediately) {
		initialize();
	}
}

void SmartLayout::initialize() {
	mSpriteMap.clear();
	clearChildren();
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand(mLayoutFile), mSpriteMap, nullptr, "", true);

	// Auto clear mNeedsLayout if client app runs layout manually
	setLayoutUpdatedFunction([this] { mNeedsLayout = false; });

	runLayout();
	mInitialized = true;
}

bool SmartLayout::hasSprite(const std::string& spriteName) { 
	return mSpriteMap.find(spriteName) != mSpriteMap.end(); 
}

ds::ui::Sprite* SmartLayout::getSprite(const std::string& spriteName) {
	auto findy = mSpriteMap.find(spriteName);
	if (findy != mSpriteMap.end()) {
		return findy->second;
	}
	return nullptr;
}

void SmartLayout::setSpriteText(const std::string& spriteName, const std::string& theText) {
	ds::ui::Text* spr = getSprite<ds::ui::Text>(spriteName);

	if (spr) {
		spr->setText(theText);
		mNeedsLayout = true;
	} else {
		DS_LOG_VERBOSE(2, "Failed to set Text for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteText(const std::string& spriteName, const std::wstring& theText) {
	return setSpriteText(spriteName, ds::utf8_from_wstr(theText));
}

void SmartLayout::setSpriteFont(const std::string& spriteName, const std::string& textCfgName) {
	ds::ui::Text* spr = getSprite<ds::ui::Text>(spriteName);

	if (spr) {
		mEngine.getEngineCfg().getText(textCfgName).configure(*spr);
		mNeedsLayout = true;
	} else {
		DS_LOG_WARNING("Failed to set Font " << textCfgName << " for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteImage(const std::string& spriteName, const std::string& imagePath) {
	ds::ui::Image* sprI = getSprite<ds::ui::Image>(spriteName);

	if (sprI) {
		sprI->setImageFile(ds::Environment::expand(imagePath));
		mNeedsLayout = true;
	} else {
		DS_LOG_VERBOSE(2, "Failed to set Image for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteImage(const std::string& spriteName, ds::Resource imageResource, bool cache) {
	ds::ui::Image* sprI = getSprite<ds::ui::Image>(spriteName);

	if (sprI) {
		if (cache) {
			sprI->setImageResource(imageResource, ds::ui::Image::IMG_CACHE_F);
		} else {
			sprI->setImageResource(imageResource);
		}
		mNeedsLayout = true;
	} else {
		DS_LOG_VERBOSE(2, "Failed to set Image for Sprite: " << spriteName);
	}
}

void SmartLayout::setSpriteTapFn(const std::string&											  spriteName,
								 const std::function<void(ds::ui::Sprite*, const ci::vec3&)>& tapCallback) {
	ds::ui::Sprite* spr = getSprite(spriteName);
	if (spr && tapCallback) {
		spr->enable(true);
		spr->setTapCallback(tapCallback);
	}
}

void SmartLayout::setContentModel(ds::model::ContentModelRef& theData) {
	mContentModel = theData;
	for (auto it : mSpriteMap) {
		auto theModel = it.second->getUserData().getString("model");
		if (!theModel.empty()) {
			auto models = ds::split(theModel, "; ", true);
			for (auto mit : models) {
				auto keyVals = ds::split(mit, ":", true);
				if (keyVals.size() == 2) {
					auto childProps = ds::split(keyVals[1], "->", true);
					if (childProps.size() == 2) {
						auto		sprPropToSet = keyVals[0];
						auto		theChild	 = childProps[0];
						auto		theProp		 = childProps[1];
						std::string actualValue  = "";

						if (sprPropToSet == "resource") {
							setSpriteImage(it.first, theData.getProperty(theProp).getResource());

						} else if (sprPropToSet == "media_player_src") {
							auto theResource = theData.getProperty(theProp).getResource();
							if(theResource.empty()) {
								theResource = ds::Resource(theData.getPropertyString(theProp));
							}
							ds::ui::XmlImporter::setSpriteProperty(
									*it.second, "media_player_src", theResource.getAbsoluteFilePath());

						} else {
							if (theChild == "this") {
								actualValue = theData.getPropertyString(theProp);
							} else {
								actualValue = theData.getChildByName(theChild).getPropertyString(theProp);
							}

							ds::ui::XmlImporter::setSpriteProperty(*it.second, sprPropToSet, actualValue);
						}

					} else {
						DS_LOG_WARNING(
								"SmartLayout::setData() Invalid syntax for child / property mapping: " << theModel);
					}
				} else {
					DS_LOG_WARNING("SmartLayout::setData() Invalid syntax for prop / model mapping: " << theModel);
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

void SmartLayout::addSpriteChild(const std::string spriteName, ds::ui::Sprite* newChild) {
	ds::ui::Sprite* spr = getSprite(spriteName);
	if (spr && newChild) {
		spr->addChildPtr(newChild);
		mNeedsLayout = true;
	} else {
		DS_LOG_WARNING("Failed to add child to " << spriteName);
	}
}

void SmartLayout::onUpdateServer(const ds::UpdateParams& p) {
	if (mNeedsLayout) {
		runLayout();
		mNeedsLayout = false;
	}
}


}  // namespace ui
}  // namespace ds
