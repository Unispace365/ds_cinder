#include "stdafx.h"

#include "launcher_local_content.h"

#include <ds/app/environment.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include "waffles/viewers/launcher/launcher_pane.h"

namespace waffles {

LauncherLocalContent::LauncherLocalContent(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/launcher/launcher_local_content.xml")
	, mMainPanel(nullptr)
	, mPaneHolder(nullptr)
	, mDrivesHolder(nullptr)
	, mDocumentsButton(nullptr)
	, mUserButton(nullptr)
	, mBackButton(nullptr)
	, mBackTitle(nullptr)
	, mBackHighTitle(nullptr)
	, mCurrentTitle(nullptr)
	, mCurrentLauncherPane(nullptr)
	, mDirectoryQuery(g, []() { return new DirectoryQuery(); }) {

	mDirectoryQuery.setReplyHandler([this](DirectoryQuery& q) { onDirectoryQuery(q); });

	mPaneHolder = getSprite("launcher_pane_holder");

	mBackButton	   = getSprite<ds::ui::LayoutButton>("back_button.the_button");
	mBackTitle	   = getSprite<ds::ui::Text>("back_button.normal_label");
	mBackHighTitle = getSprite<ds::ui::Text>("back_button.high_label");
	mCurrentTitle  = getSprite<ds::ui::Text>("current_title");
	mMainPanel	   = getSprite<ds::ui::LayoutSprite>("main_panel");

	mDocumentsButton = setupDriveButton("documents.the_button", "%DOCUMENTS%/");
	// auto userButton = setupDriveButton("user.the_button", "C:/");

	mDrivesHolder = getSprite("drives_holder");
	if (mDrivesHolder) {
		auto localLocationCount = mEngine.getWafflesSettings().countSetting("local_files:path");
		for (int i = 0; i < localLocationCount; ++i) {
			auto nameAndPath =
				ds::split(mEngine.getWafflesSettings().getString("local_files:path", i, "Documents;%DOCUMENTS%/"), ";");
			if (nameAndPath.size() != 2) continue;

			std::map<std::string, ds::ui::Sprite*> driveSpriteMap;
			ds::ui::XmlImporter::loadXMLto(
				mDrivesHolder, ds::Environment::expand("%APP%/data/layouts/waffles/launcher/launcher_type_button.xml"),
				driveSpriteMap);

			auto theButton = dynamic_cast<ds::ui::LayoutButton*>(driveSpriteMap["the_button"]);
			auto normLabel = dynamic_cast<ds::ui::Text*>(driveSpriteMap["normal_label"]);
			auto highLabel = dynamic_cast<ds::ui::Text*>(driveSpriteMap["high_label"]);
			auto normIcon  = dynamic_cast<ds::ui::Image*>(driveSpriteMap["normal_icon"]);
			auto highIcon  = dynamic_cast<ds::ui::Image*>(driveSpriteMap["high_icon"]);
			if (theButton && normIcon && highIcon && normLabel && highLabel) {
				normLabel->setText(nameAndPath[0]);
				highLabel->setText(nameAndPath[0]);
				normIcon->setImageFile(ds::Environment::expand("%APP%/data/images/waffles/icons/1x/Folder_64.png"));
				highIcon->setImageFile(ds::Environment::expand("%APP%/data/images/waffles/icons/1x/Folder_64.png"));
				std::string directoryPath = nameAndPath[1];
				theButton->setClickFn([this, theButton, directoryPath] { buttonClicked(theButton, directoryPath); });
				mDriveButtons.push_back(theButton);
			}
		}
	}

	if (mBackButton) {
		mBackButton->setClickFn([this] {
			if (mHistory.empty() || mHistory.size() < 2) return;

			mHistory.pop_back();
			auto backy = mHistory.back();
			requestPaneData(backy.getPropertyString("path"));
		});

		mBackButton->hide();
	}

	const float startWidth	= getWidth();
	const float startHeight = getHeight();

	setSize(startWidth, startHeight);
	mLayoutUserType = ds::ui::LayoutSprite::kFillSize;

	runLayout();
}


void LauncherLocalContent::selectDocuments() {
	if (mDocumentsButton) {
		buttonClicked(mDocumentsButton, "%DOCUMENTS%");
	}
}

ds::ui::LayoutButton* LauncherLocalContent::setupDriveButton(const std::string& buttonName,
															 const std::string& directoryPath) {
	ds::ui::LayoutButton* sb = getSprite<ds::ui::LayoutButton>(buttonName);
	if (!sb) {
		return nullptr;
	}

	sb->setClickFn([this, sb, directoryPath] { buttonClicked(sb, directoryPath); });
	mDriveButtons.push_back(sb);
	return sb;
}


void LauncherLocalContent::buttonClicked(ds::ui::LayoutButton* sb, const std::string& directoryPath) {
	for (auto it : mDriveButtons) {
		it->showUp();
	}

	if (sb) {
		sb->showDown();
	}

	mHistory.clear();

	requestPaneData(ds::Environment::expand(directoryPath));
}

void LauncherLocalContent::onDirectoryQuery(DirectoryQuery& q) {
	setPaneData(q.mOutput, q.mError);
}

void LauncherLocalContent::requestPaneData(const std::string& panePath) {
	mDirectoryQuery.start([panePath](DirectoryQuery& q) { q.setInputType(false), q.setDirectoryToLoad(panePath, 0); },
						  false);

	if (mCurrentLauncherPane) {
		auto clp = mCurrentLauncherPane;
		mCurrentLauncherPane->tweenPosition(
			ci::vec3(-mCurrentLauncherPane->getWidth(), mCurrentLauncherPane->getPosition().y, 0.0f),
			mEngine.getAnimDur(), 0.0f, ci::easeInQuad, [clp] { clp->release(); });
		mCurrentLauncherPane = nullptr;
	}
}

void LauncherLocalContent::setPaneData(ds::model::ContentModelRef newData, const bool isError) {
	if (mCurrentLauncherPane) {
		auto clp = mCurrentLauncherPane;
		mCurrentLauncherPane->tweenPosition(
			ci::vec3(-mCurrentLauncherPane->getWidth(), mCurrentLauncherPane->getPosition().y, 0.0f),
			mEngine.getAnimDur(), 0.0f, ci::easeInQuad, [clp] { clp->release(); });
		mCurrentLauncherPane = nullptr;
	}

	if (!mPaneHolder) {
		DS_LOG_WARNING("No pane, no gain.");
		return;
	}

	if (mHistory.empty() || mHistory.back().getPropertyString("path") != newData.getPropertyString("path")) {
		mHistory.push_back(newData);
	}
	if (mCurrentTitle) {
		auto thisPanel = newData.getPropertyString("name");
		if (thisPanel.empty()) {
			auto drives = mEngine.mContent.getChildByName("physical_drives");
			for (auto it : drives.getChildren()) {
				if (newData.getPropertyString("path") == it.getPropertyString("path")) {
					thisPanel = it.getPropertyString("name");
					break;
				}
			}
		}
		mCurrentTitle->setText(thisPanel);
		if (mMainPanel) {
			mMainPanel->runLayout();
		}
		mCurrentTitle->tweenAnimateOn(true, 0.0f, 0.05f);
	}

	if (mBackButton && mBackTitle && mBackHighTitle) {
		if (mHistory.size() < 2) {
			mBackButton->hide();
		} else {
			mBackButton->show();
			mBackButton->tweenAnimateOn(true, 0.0f, 0.05f);

			auto granny		   = mHistory[mHistory.size() - 2];
			auto backTitleText = granny.getPropertyString("name");
			if (backTitleText.empty()) {
				auto drives = mEngine.mContent.getChildByName("physical_drives");
				for (auto it : drives.getChildren()) {
					if (granny.getPropertyString("path") == it.getPropertyString("path")) {
						backTitleText = it.getPropertyString("name");
						break;
					}
				}
			}

			mBackTitle->setText(backTitleText);
			mBackHighTitle->setText(backTitleText);
		}
	}

	mCurrentLauncherPane = new LauncherPane(mEngine);
	mPaneHolder->addChildPtr(mCurrentLauncherPane);
	mCurrentLauncherPane->setFolderBrowseCallback(
		[this](ds::model::ContentModelRef media) { requestPaneData(media.getPropertyString("path")); });
	mCurrentLauncherPane->setSizeAll(mPaneHolder->getSize());
	mCurrentLauncherPane->setData(newData.getChildren());
	mCurrentLauncherPane->animateOn();
}

} // namespace waffles
