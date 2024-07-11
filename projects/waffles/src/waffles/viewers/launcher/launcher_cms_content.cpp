#include "stdafx.h"

#include "launcher_cms_content.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/file_meta_data.h>
#include <ds/util/string_util.h>

#include <ds/data/resource.h>

#include "app/app_defs.h"
//#include "events/app_events.h"

#include "waffles/viewers/launcher/launcher_pane.h"

namespace waffles {

LauncherCmsContent::LauncherCmsContent(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/launcher/launcher_cms_content.xml") {

	mPaneHolder = getSprite("launcher_pane_holder");

	mRecentButton		 = setupTypeButton("recent_files");
	mFoldersButton		 = setupTypeButton("folders");
	mPinboardsButton	 = setupTypeButton("pinboards");
	mVideoButton		 = setupTypeButton("videos");
	mStreamButton		 = setupTypeButton("streams");
	mPdfButton			 = setupTypeButton("pdfs");
	mImageButton		 = setupTypeButton("images");
	mWebButton			 = setupTypeButton("web");
	mPresentationsButton = setupTypeButton("presentations");

	mBackButton	   = getSprite<ds::ui::LayoutButton>("back_button.the_button");
	mBackTitle	   = getSprite<ds::ui::Text>("back_button.normal_label");
	mBackHighTitle = getSprite<ds::ui::Text>("back_button.high_label");
	mCurrentTitle  = getSprite<ds::ui::Text>("current_title");

	if (mBackButton) {
		mBackButton->setClickFn([this] {
			ds::model::ContentModelRef parentRef;
			auto					   cmsContent = mEngine.mContent.getChildByName("cms_root");

			if (mBackMediaRef.getName() == "cms_root") {
				setPaneData(cmsContent.getChildren(), false);
				return;
			}

			// If the current launcher pane has no items, used the cached back ref
			// This gets the parent of the current view
			if (mCurrentLauncherPane && !mCurrentLauncherPane->getBackMediaModel(parentRef)) {
				parentRef = mBackMediaRef;
			}

			if (parentRef.getId() < 1) {
				// DS_LOG_WARNING("Data issue trying to go back! Likely requeried in the meantime");
				// return;
			}

			// Find the grandbackparent, the parent of the previous view
			ds::model::ContentModelRef grandParentRef;
			if (parentRef.getId() > 0) {
				grandParentRef = cmsContent.getDescendant("waffles_nodes", parentRef.getPropertyInt("parent_id"));
			}
			if (!grandParentRef.empty()) {
				bool					   canBack = false;
				ds::model::ContentModelRef greatGrandParent =
					cmsContent.getDescendant("waffles_nodes", grandParentRef.getPropertyInt("parent_id"));
				if (!grandParentRef.empty()) {
					canBack = true;
				} else {
					auto events =
						mEngine.mContent.getChildByName("cms_current_events").getChildren(); // printTree(true);
					std::vector<ds::model::ContentModelRef> folders;
					for (auto event : events) {
						for (auto id : event.getPropertyListInt("selected_content_node_ids")) {
							if (auto nodey =
									mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", id)) {
								folders.push_back(nodey);
							}
						}
					}

					auto folderParent = ds::model::ContentModelRef();
					folderParent.setProperty("name", std::string("Folders"));
					folderParent.setChildren(folders);
					grandParentRef = folderParent;
					canBack		   = true;
				}

				setPaneData(grandParentRef.getChildren(), canBack, grandParentRef);
			} else {
				auto events = mEngine.mContent.getChildByName("cms_current_events").getChildren(); // printTree(true);
				std::vector<ds::model::ContentModelRef> folders;
				for (auto event : events) {
					for (auto id : event.getPropertyListInt("selected_content_node_ids")) {
						if (auto nodey =
								mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", id)) {
							folders.push_back(nodey);
						}
					}
				}

				auto folderParent = ds::model::ContentModelRef();
				folderParent.setChildren(folders);

				setPaneData(folders, false, folderParent);
			}
		});

		mBackButton->hide();
	}

	runLayout();

	callAfterDelay(
		[this] {
			ds::ui::Text* normalLabel =
				dynamic_cast<ds::ui::Text*>(mRecentButton->getFirstDescendantWithName(L"recents.normal_label"));
			std::string theTitle = "Recent";
			if (normalLabel) theTitle = normalLabel->getTextAsString();
			buttonClicked(mRecentButton, theTitle);
		},
		mEngine.getAnimDur());


	const float startWidth	= getWidth();
	const float startHeight = getHeight();

	setSize(startWidth, startHeight);
}


void LauncherCmsContent::showPresentations() {
	if (!mPresentationsButton) return;
	ds::ui::Text* normalLabel =
		dynamic_cast<ds::ui::Text*>(mPresentationsButton->getFirstDescendantWithName(L"recents.normal_label"));
	std::string theTitle = "Presentations";
	if (normalLabel) theTitle = normalLabel->getTextAsString();
	buttonClicked(mPresentationsButton, theTitle);
}

void LauncherCmsContent::showRecents() {
	if (!mRecentButton) return;
	ds::ui::Text* normalLabel =
		dynamic_cast<ds::ui::Text*>(mRecentButton->getFirstDescendantWithName(L"recents.normal_label"));
	std::string theTitle = "Recent";
	if (normalLabel) theTitle = normalLabel->getTextAsString();
	buttonClicked(mRecentButton, theTitle);
}

ds::ui::LayoutButton* LauncherCmsContent::setupTypeButton(const std::string& buttonName) {
	auto sb = getSprite<ds::ui::LayoutButton>(buttonName + ".the_button");
	if (!sb) {
		DS_LOG_WARNING("Launcher CMS button not found! " << buttonName);
		return nullptr;
	}

	sb->setClickFn([this, sb, buttonName] {
		auto normalLabel = getSprite<ds::ui::Text>(buttonName + ".normal_label");
		if (normalLabel && sb) {
			buttonClicked(sb, normalLabel->getTextAsString());
		}
	});

	mTypeButtons.push_back(sb);
	return sb;
}


void LauncherCmsContent::buttonClicked(ds::ui::LayoutButton* sb, const std::string& panelTitle) {
	for (auto it : mTypeButtons) {
		it->showUp();
	}

	if (sb) {
		sb->showDown();
	}

	if (sb == mRecentButton) {
		auto activeNodes = mEngine.mContent.getChildByName("cms_current_events").getPropertyListInt("active_nodes");

		auto									cmsContent = mEngine.mContent.getChildByName("cms_root");
		std::vector<ds::model::ContentModelRef> recents = mEngine.mContent.getChildByName("recent_files").getChildren();
		std::vector<ds::model::ContentModelRef> allowedy;
		for (auto it : recents) {
			if (std::find(activeNodes.begin(), activeNodes.end(), it.getId()) != activeNodes.end()) {
				if (it.getPropertyString("type") == MEDIA_TYPE_FILE_CMS || it.getPropertyString("type") == MEDIA_TYPE_PRESENTATION ) {
					auto theThing = cmsContent.getDescendant("waffles_nodes", it.getId());
					if (theThing.empty()) continue;
					theThing.setProperty("last_opened", it.getPropertyString("last_opened"));
					allowedy.emplace_back(theThing);
				} else if (ds::safeFileExistsCheck(
							   ds::Environment::expand(it.getPropertyResource("media_res").getAbsoluteFilePath()))) {
					allowedy.emplace_back(it);
				}
			}
		}


		std::sort(allowedy.begin(), allowedy.end(),
				  [](ds::model::ContentModelRef& a, ds::model::ContentModelRef& b) -> bool {
					  return a.getPropertyString("last_opened") > b.getPropertyString("last_opened");
				  });

		ds::model::ContentModelRef titleRef;
		titleRef.setProperty("name", panelTitle);
		setPaneData(allowedy, false, titleRef);

	} else if (sb == mFoldersButton) {
		auto events = mEngine.mContent.getChildByName("cms_current_events").getChildren(); // printTree(true);
		std::vector<ds::model::ContentModelRef> folders;
		for (auto event : events) {
			for (auto id : event.getPropertyListInt("selected_content_node_ids")) {
				if (auto nodey = mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", id)) {
					folders.push_back(nodey);
				}
			}
		}

		auto folderParent = ds::model::ContentModelRef();
		folderParent.setChildren(folders);

		setPaneData(folders, false, folderParent);
	} else if (sb == mVideoButton) {
		setPaneData(std::vector<int>{ds::Resource::VIDEO_TYPE, ds::Resource::YOUTUBE_TYPE}, panelTitle);
	} else if (sb == mStreamButton) {
		setPaneData(ds::Resource::VIDEO_STREAM_TYPE, panelTitle);
	} else if (sb == mPresetsButton) {
		setPaneData(MEDIA_TYPE_PRESET, panelTitle);
	} else if (sb == mPresentationsButton) {
		setPaneData(MEDIA_TYPE_PRESENTATION, panelTitle);
	} else if (sb == mWebButton) {
		setPaneData(ds::Resource::WEB_TYPE, panelTitle);
	} else if (sb == mImageButton) {
		setPaneData(ds::Resource::IMAGE_TYPE, panelTitle);
	} else if (sb == mPdfButton) {
		setPaneData(ds::Resource::PDF_TYPE, panelTitle);
	} else if (sb == mPinboardsButton) {
		auto pinboard = mEngine.mContent.getChildByName("current_pinboard"); // printTree(true);
		std::vector<ds::model::ContentModelRef> items;
		for (auto&& [id, node] : pinboard.getReferences("pinboard_items")) {
			items.push_back(node);
		}

		auto fakeParent = ds::model::ContentModelRef();
		fakeParent.setChildren(items);

		setPaneData(items, false, fakeParent);
	}
}

void LauncherCmsContent::getContentOfType(const std::string& mediaType, ds::model::ContentModelRef parentNode,
										  std::vector<ds::model::ContentModelRef>& outVec) {

	for (auto it : parentNode.getChildren()) {
		if (it.getPropertyString("type") == mediaType && it.getName() == "waffles_nodes") {
			outVec.emplace_back(it);
		}

		getContentOfType(mediaType, it, outVec);
	}
}

void LauncherCmsContent::getContentOfResourceType(const int& recType, ds::model::ContentModelRef parentNode,
												  std::vector<ds::model::ContentModelRef>& outVec) {

	for (auto it : parentNode.getChildren()) {
		if (it.getPropertyResource("media_res").getType() == recType && it.getName() == "waffles_nodes") {
			outVec.emplace_back(it);
		}

		getContentOfResourceType(recType, it, outVec);
	}
}

void LauncherCmsContent::getContentOfType(const std::string&					   mediaType,
										  std::vector<ds::model::ContentModelRef>& parentNodes,
										  std::vector<ds::model::ContentModelRef>& outVec) {

	for (auto parentNode : parentNodes) {
		for (auto it : parentNode.getChildren()) {
			if (it.getPropertyString("type") == mediaType && it.getName() == "waffles_nodes") {
				outVec.emplace_back(it);
			}

			getContentOfType(mediaType, it, outVec);
		}
	}
}

void LauncherCmsContent::getContentOfResourceType(const int&							   recType,
												  std::vector<ds::model::ContentModelRef>& parentNodes,
												  std::vector<ds::model::ContentModelRef>& outVec) {

	for (auto parentNode : parentNodes) {
		for (auto it : parentNode.getChildren()) {
			if (it.getPropertyResource("media_res").getType() == recType && it.getName() == "waffles_nodes") {
				outVec.emplace_back(it);
			}

			getContentOfResourceType(recType, it, outVec);
		}
	}
}

void LauncherCmsContent::sortMediaAlpha(std::vector<ds::model::ContentModelRef>& theMedia) {
	std::sort(theMedia.begin(), theMedia.end(),
			  [](ds::model::ContentModelRef& a, ds::model::ContentModelRef& b) -> bool {
				  auto strA = a.getPropertyString("name");
				  ds::to_lowercase(strA);
				  auto strB = b.getPropertyString("name");
				  ds::to_lowercase(strB);
				  return strA < strB;
			  });
}

void LauncherCmsContent::setPaneData(const std::string& mediaType, const std::string& title) {

	auto events = mEngine.mContent.getChildByName("cms_current_events").getChildren();
	std::vector<ds::model::ContentModelRef> folders;
	for (auto event : events) {
		for (auto id : event.getPropertyListInt("selected_content_node_ids")) {
			if (auto nodey = mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", id)) {
				folders.push_back(nodey);
			}
		}
	}

	std::vector<ds::model::ContentModelRef> theMedia;
	getContentOfType(mediaType, folders, theMedia);

	sortMediaAlpha(theMedia);

	ds::model::ContentModelRef backModel;
	backModel.setProperty("name", title);

	setPaneData(theMedia, false, backModel);
}

void LauncherCmsContent::setPaneData(const int resourceType, const std::string& title) {

	auto events = mEngine.mContent.getChildByName("cms_current_events").getChildren();
	std::vector<ds::model::ContentModelRef> folders;
	for (auto event : events) {
		for (auto id : event.getPropertyListInt("selected_content_node_ids")) {
			if (auto nodey = mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", id)) {
				folders.push_back(nodey);
			}
		}
	}

	std::vector<ds::model::ContentModelRef> theMedia;
	getContentOfResourceType(resourceType, folders, theMedia);

	sortMediaAlpha(theMedia);

	ds::model::ContentModelRef backModel;
	backModel.setProperty("name", title);

	setPaneData(theMedia, false, backModel);
}

void LauncherCmsContent::setPaneData(std::vector<int> resourceTypes, const std::string& title) {

	auto events = mEngine.mContent.getChildByName("cms_current_events").getChildren();
	std::vector<ds::model::ContentModelRef> folders;
	for (auto event : events) {
		for (auto id : event.getPropertyListInt("selected_content_node_ids")) {
			if (auto nodey = mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", id)) {
				folders.push_back(nodey);
			}
		}
	}

	std::vector<ds::model::ContentModelRef> theMedia;
	for (auto resourceType : resourceTypes) {
		getContentOfResourceType(resourceType, folders, theMedia);
	}

	sortMediaAlpha(theMedia);

	ds::model::ContentModelRef backModel;
	backModel.setProperty("name", title);

	setPaneData(theMedia, false, backModel);
}

void LauncherCmsContent::setPaneData(std::vector<ds::model::ContentModelRef> newData, const bool canBack,
									 ds::model::ContentModelRef backModel) {
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

	if (mCurrentTitle) {
		mCurrentTitle->setText(backModel.getPropertyString("name"));
		mCurrentTitle->tweenAnimateOn(true, 0.0f, 0.025f);
	}

	if (mBackButton && mBackTitle && mBackHighTitle) {
		
		bool					   foundGrandParent = false;
		ds::model::ContentModelRef grandParentRef =
			mEngine.mContent.getChildByName("cms_root")
				.getDescendant("waffles_nodes", backModel.getPropertyInt("parent_id"));

		if (!grandParentRef.empty()) {
			foundGrandParent = true;
		}

		if (!foundGrandParent) {
			auto events = mEngine.mContent.getChildByName("cms_current_events").getChildren(); // printTree(true);
			std::vector<ds::model::ContentModelRef> folders;
			for (auto event : events) {
				for (auto id : event.getPropertyListInt("selected_content_node_ids")) {
					if (auto nodey = mEngine.mContent.getChildByName("cms_root").getDescendant("waffles_nodes", id)) {
						folders.push_back(nodey);
					}
				}
			}

			auto folderParent = ds::model::ContentModelRef();
			folderParent.setProperty("name", std::string("Folders"));
			folderParent.setChildren(folders);
			grandParentRef = folderParent;
			// grandParentRef = mEngine.mContent.getChildByName("cms_root");
		}

		if (canBack) {
			mBackButton->show();
			mBackButton->tweenAnimateOn(true, 0.0f, 0.025f);
			mBackTitle->setText(grandParentRef.getPropertyString("name"));
			mBackHighTitle->setText(grandParentRef.getPropertyString("name"));
			mBackMediaRef = grandParentRef;


			for (auto it : mTypeButtons) {
				it->showUp();
			}

			if (mFoldersButton) {
				mFoldersButton->showDown();
			}

		} else {
			mBackButton->hide();
		}
	}

	mCurrentLauncherPane = new LauncherPane(mEngine);
	mPaneHolder->addChildPtr(mCurrentLauncherPane);
	mCurrentLauncherPane->setFolderBrowseCallback(
		[this](ds::model::ContentModelRef media) { setPaneData(media.getChildren(), true, media); });
	mCurrentLauncherPane->setSizeAll(mPaneHolder->getSize());
	mCurrentLauncherPane->setData(newData);
	mCurrentLauncherPane->animateOn();
}

} // namespace waffles
