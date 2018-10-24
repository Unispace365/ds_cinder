#include "stdafx.h"

#include "smart_layout.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>
#include <ds/util/string_util.h>
#include <ds/ui/scroll/smart_scroll_list.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/button/sprite_button.h>


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
			sprI->setImageResource(imageResource, ds::ui::Image::IMG_CACHE_F);// | ds::ui::Image::IMG_ENABLE_MIPMAP_F);
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

void SmartLayout::setSpriteClickFn(const std::string& spriteName, const std::function<void()>& clickCallback) {
	if(!clickCallback) return;
	auto ib = getSprite<ds::ui::ImageButton>(spriteName);
	if(ib) {
		ib->setClickFn(clickCallback);
		return;
	}
	auto lb = getSprite<ds::ui::LayoutButton>(spriteName);
	if(lb) {
		lb->setClickFn(clickCallback);
		return;
	}
	auto sb = getSprite<ds::ui::SpriteButton>(spriteName);
	if(sb) {
		sb->setClickFn(clickCallback);
		return;
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

						ds::model::ContentModelRef theNode = theData;
						if(theChild != "this") {
							theNode = theData.getChildByName(theChild);
						}
						if(sprPropToSet == "smart_scroll_children") {
							auto ssl = getSprite<SmartScrollList>(it.first);
							if(ssl) {
								ssl->setContentList(theNode);
							}
						} else if (sprPropToSet == "resource") {
							setSpriteImage(it.first, theNode.getProperty(theProp).getResource());
						} else if (sprPropToSet == "resource_cache") {
							setSpriteImage(it.first, theNode.getProperty(theProp).getResource(), true);
						} else if(sprPropToSet == "media_player_src") {
							auto theResource = theNode.getProperty(theProp).getResource();
							if(theResource.empty()) {
								theResource = ds::Resource(ds::Environment::expand(theNode.getPropertyString(theProp)));
							} else if(theResource.getType() == ds::Resource::IMAGE_TYPE) {
								ds::ImageMetaData metaData;
								metaData.add(ds::Environment::expand(theResource.getAbsoluteFilePath()), ci::vec2(theResource.getWidth(), theResource.getHeight()));
							}
							if(!theResource.empty()) {
								ds::ui::XmlImporter::setSpriteProperty(
									*it.second, "media_player_src", theResource.getAbsoluteFilePath());
							}

						} else {
							actualValue = theNode.getPropertyString(theProp);

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

	runLayout();

	if(mContentUpdatedCallback) { mContentUpdatedCallback(); }
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
