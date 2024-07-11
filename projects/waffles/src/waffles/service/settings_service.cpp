#include "stdafx.h"

#include "settings_service.h"

#include <fstream>

#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/util/file_meta_data.h>

#include "app/app_defs.h"
#include "waffles/waffles_events.h"

namespace waffles {

SettingsService::SettingsService(ds::ui::SpriteEngine& g)
	: mSettingsFilePath("%LOCAL%/waffles-neu/settings.xml")
	, mEngine(g)
	, mEventClient(g)
	, mSaveService(g, []() { return new FileWriteRunnable(); })
	, mInitialized(false) {

	mEventClient.listenToEvents<BackgroundChangeComplete>([this](auto& e) { saveSettings(); });
	mEventClient.listenToEvents<AmbientSettingsUpdated>([this](auto& e) { saveSettings(); });
	mEventClient.listenToEvents<LockoutSettingsUpdated>([this](auto& e) { saveSettings(); });
	mEventClient.listenToEvents<CmsDataLoadCompleteEvent>([this](auto& e) { initialize(); });
}

void SettingsService::initialize() {
	if (mInitialized) return;
	mInitialized = true;

	try {

		std::string fileName = ds::Environment::expand("%LOCAL%/waffles-neu/settings.xml");
		if (!ds::safeFileExistsCheck(fileName)) {
			DS_LOG_WARNING("No settings file found, using defaults.");
			mEventClient.notify(SettingsInitializeComplete());
			return;
		}

		cinder::XmlTree xml(cinder::loadFile(fileName));

		if (xml.hasChild("ambient") && xml.hasChild("ambient_custom")) {
			ci::XmlTree ambientCusomt	  = xml.getChild("ambient_custom");
			auto&		modelAmbient	  = mEngine.mContent.getChildByName("ambient");
			auto		modelAmbientMedia = ds::model::ContentModelRef();
			readMediaItem(ambientCusomt, modelAmbientMedia);

			modelAmbient.clearChildren();
			modelAmbient.addChild(modelAmbientMedia);

			ci::XmlTree amb = xml.getChild("ambient");

			if (amb.hasAttribute("type")) {
				auto ambType = amb.getAttributeValue<std::string>("type");
				modelAmbient.setProperty("type", ambType);
			}

			if (amb.hasAttribute("show_ui")) {
				auto hasUi = amb.getAttributeValue<std::string>("show_ui");
				modelAmbient.setProperty("show_title", hasUi);
			}

			if (amb.hasAttribute("timeout")) {
				int engineIdle = ds::string_to_int(amb.getAttributeValue<std::string>("timeout"));
				if (engineIdle < 5) engineIdle = 5;
				mEngine.setIdleTimeout(engineIdle);

				ds::Engine* engey = dynamic_cast<ds::Engine*>(&mEngine);
				if (engey) {

					const size_t numRoots = engey->getRootCount();
					for (size_t i = 0; i < numRoots - 1; i++) {
						if (engey->getRootBuilder(i).mDebugDraw) continue;
						engey->getRootSprite(i).setSecondBeforeIdle(mEngine.getIdleTimeout());
					}
				}
			}

			if (amb.hasAttribute("slide_seconds")) {
				float slideSecs = amb.getAttributeValue<float>("slide_seconds");
				modelAmbient.setProperty("slide_seconds", slideSecs);
			}
		}

		if (xml.hasChild("user_background")) {
			ci::XmlTree userMedia	= xml.getChild("user_background");
			std::string mediaPath		= userMedia.getValue<std::string>();
			auto		userContent = mEngine.mContent.getChildByName("background.user");
			auto userModel = ds::model::ContentModelRef();
			if (userMedia.hasAttribute("pdf_page")) {
				userModel.setProperty("pdf_page", userMedia.getAttribute("pdf_page"));
			}
			readMediaItem(userMedia, userModel);
			//userModel.setPropertyResource("media_res", ds::Resource(mediaPath));
			userContent.clearChildren();
			userContent.addChild(userModel);
		}

		if (xml.hasChild("particle_media")) {
			ci::XmlTree particleMedia	= xml.getChild("particle_media");
			std::string mediaPath		= particleMedia.getValue<std::string>();
			auto		particleContent = mEngine.mContent.getChildByName("background.particle");
			if (particleMedia.hasAttribute("pdf_page")) {
				particleContent.setProperty("pdf_page", particleMedia.getAttribute("pdf_page"));
			}
			particleContent.setPropertyResource("media_res", ds::Resource(mediaPath));
		}

		updateBackground();

	} catch (std::exception& e) {
		DS_LOG_WARNING("Exception reading settings: " << e.what());
	}

	mEventClient.notify(SettingsInitializeComplete());
}

void SettingsService::updateBackground() {
	std::string fileName = ds::Environment::expand(mSettingsFilePath);
	if (!ds::safeFileExistsCheck(fileName)) return;
	cinder::XmlTree xml(cinder::loadFile(fileName));

	if (xml.hasChild("current_background")) {
		ci::XmlTree currBackground = xml.getChild("current_background");
		int			backgroundType = ds::string_to_int(currBackground.getAttribute("type").getValue());

		auto userBackground	 = mEngine.mContent.getChildByName("background.user");
		auto modelBackground = ds::model::ContentModelRef();
		int	 pdfPage		 = 0;
		if (xml.hasChild("user_background")) {
			ci::XmlTree userBackgroundXml = xml.getChild("user_background");
			if (userBackgroundXml.hasAttribute("pdf_page")) {
				pdfPage = ds::string_to_int(userBackgroundXml.getAttribute("pdf_page"));
				userBackground.setProperty("pdf_page", pdfPage);
			}
			readMediaItem(userBackgroundXml, modelBackground);
		}

		if (backgroundType == BACKGROUND_TYPE_USER_MEDIA) {
			mEngine.getNotifier().notify(RequestBackgroundChange(backgroundType, modelBackground, pdfPage));
		} else if (backgroundType == BACKGROUND_TYPE_DEFAULT) {
			mEngine.getNotifier().notify(
				RequestBackgroundChange(backgroundType, mEngine.mContent.getChildByName("background.default")));

			// save the user background for later even though we're not using it right now
			userBackground.clearChildren();
			userBackground.addChild(modelBackground);
		} else {
			mEngine.getNotifier().notify(RequestBackgroundChange(backgroundType, ds::model::ContentModelRef()));

			// save the user background for later even though we're not using it right now
			userBackground.clearChildren();
			userBackground.addChild(modelBackground);
		}
	}
}

void SettingsService::readMediaItem(ci::XmlTree& theTree, ds::model::ContentModelRef& outputMedia) {
	std::string userType = theTree.getAttributeValue<std::string>("type");
	if (userType == MEDIA_TYPE_FILE_CMS || userType == MEDIA_TYPE_PRESENTATION ||
		userType == MEDIA_TYPE_DIRECTORY_CMS || userType == MEDIA_TYPE_PRESET) {
		int mediaId = ds::string_to_int(theTree.getAttributeValue<std::string>("id"));
		outputMedia = mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", mediaId);
	} else {
		std::string theTitle = theTree.getAttributeValue<std::string>("name");
		std::string filePath = theTree.getValue<std::string>();
		outputMedia.setId(0);
		outputMedia.setProperty("type", MEDIA_TYPE_FILE_LOCAL);
		outputMedia.setProperty("name", theTitle);
		outputMedia.setPropertyResource("media_res", ds::Resource(filePath));
	}
}

void SettingsService::writeMediaItem(ci::XmlTree& theTree, ds::model::ContentModelRef inputMedia) {
	theTree.setAttribute("id", inputMedia.getId());
	theTree.setAttribute("type", inputMedia.getPropertyString("type"));
	theTree.setAttribute("name", inputMedia.getPropertyString("name"));
	theTree.setValue(inputMedia.getPropertyResource("media_res").getAbsoluteFilePath());
}

void SettingsService::saveSettings() {

	// Prevent saving the defaults if we haven't loaded data yet
	if (!mInitialized) return;

	ci::XmlTree theDoc = ci::XmlTree::createDoc();
	ci::XmlTree backgroundType;
	backgroundType.setTag("current_background");
	backgroundType.setAttribute("type",
								mEngine.mContent.getChildByName("background.current").getPropertyString("type"));
	theDoc.push_back(backgroundType);
	if (!mEngine.mContent.getChildByName("background.user").getChildren().empty()) {

		ci::XmlTree background;
		background.setTag("user_background");
		background.setAttribute("pdf_page",
								mEngine.mContent.getChildByName("background.user").getPropertyString("pdf_page"));
		writeMediaItem(background, mEngine.mContent.getChildByName("background.user").getChild(0));

		theDoc.push_back(background);
	}

	auto		ambientModel = mEngine.mContent.getChildByName("ambient");
	ci::XmlTree ambientTree;
	ambientTree.setTag("ambient");
	ambientTree.setAttribute("show_ui", ambientModel.getPropertyString("show_title"));
	ambientTree.setAttribute("type", ambientModel.getPropertyString("type"));
	ambientTree.setAttribute("timeout", mEngine.getIdleTimeout());
	ambientTree.setAttribute("slide_seconds", ambientModel.getPropertyString("slide_seconds"));

	theDoc.push_back(ambientTree);

	ci::XmlTree ambientCustom;
	ambientCustom.setTag("ambient_custom");
	writeMediaItem(ambientCustom, ambientModel.getChild(0));
	theDoc.push_back(ambientCustom);

	auto		particleModel = mEngine.mContent.getChildByName("background.particle");
	ci::XmlTree particleBackground;
	particleBackground.setTag("particle_media");
	particleBackground.setValue(particleModel.getPropertyResource("media_res").getAbsoluteFilePath());
	particleBackground.setAttribute("pdf_page", particleModel.getPropertyString("pdf_page"));
	theDoc.push_back(particleBackground);


	mSaveService.start([this, theDoc](FileWriteRunnable& q) { q.setSaveableXml(theDoc); });
}

SettingsService::FileWriteRunnable::FileWriteRunnable() {
}

void SettingsService::FileWriteRunnable::setSaveableXml(const ci::XmlTree& theData) {
	mData = theData;
}

void SettingsService::FileWriteRunnable::run() {
	mData.write(ci::writeFile(ds::Environment::expand("%LOCAL%/waffles-neu/settings.xml"), true));
}
} // namespace waffles
