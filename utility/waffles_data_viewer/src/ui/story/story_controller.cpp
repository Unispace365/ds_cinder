#include "stdafx.h"

#include "story_controller.h"
#include "story_view.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/engine/engine_events.h>
#include <ds/content/content_events.h>
#include <ds/ui/scroll/smart_scroll_list.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/media/media_player.h>
#include <ds/util/string_util.h>
#include <Poco/Path.h>

#include "events/app_events.h"

#include <shellapi.h>

namespace downstream {

StoryController::StoryController(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "story_controller.xml")
{


	// NOTE: this event gets called anytime ANY data has changed, so use with discretion
	// If content got updated in other parts of the app (or other data not even related to this app)
	// this will still be called, so be sure it only changes when needed
	listenToEvents<CmsDataLoadCompleteEvent>([this](const CmsDataLoadCompleteEvent& e) {
		auto theRoot = mEngine.mContent.getChildByName("cms_root");
		if (!mInitialized && theRoot.hasChildren()) {
			setData(theRoot);
			mInitialized = true;
			auto ni = getSprite("no_items");
			if (ni)ni->hide();
		}
	});

	setSpriteTapFn("bread_crumbs", [this](ds::ui::Sprite*, const ci::vec3&) {
		goBack();
	});

	setSpriteClickFn("back_button.the_button", [this] {
		goBack();
	});

	setSpriteClickFn("top_button.the_button", [this] {
		auto theRoot = mEngine.mContent.getChildByName("cms_root");
		setData(theRoot);
	});

	auto fileList = getSprite<ds::ui::SmartScrollList>("scroll_list");
	if (!fileList) {
		DS_LOG_WARNING("No file scroll list found for search viewer");
		return;
	}

	fileList->setContentItemTappedCallback([this](ds::ui::SmartLayout* sl, ds::model::ContentModelRef theModel) {
		if (theModel.hasChildren()) {
			setData(theModel);
		} else {
			showInfoPage(theModel);
		}
	});

	fileList->setContentItemUpdatedCallback([this, fileList](ds::ui::SmartLayout* sl) {
		auto cm = sl->getContentModel();
		sl->setSpriteClickFn("info_button", [this, sl] {
			showInfoPage(sl->getContentModel());
		});

		auto nextBtn = sl->getSprite("next_button");
		if (nextBtn) {
			if (cm.hasChildren()) {
				nextBtn->show();
			} else {
				nextBtn->hide();
			}
		}
		sl->runLayout();
	});

	auto scrollBar = getSprite<ds::ui::ScrollBar>("scroll_bar");
	if (scrollBar) {
		scrollBar->linkScrollList(fileList);
		ci::ColorA nubbinColor = mEngine.getColors().getColorFromName("ui_highlight");
		scrollBar->getNubSprite()->setColor(nubbinColor);
		scrollBar->getBackgroundSprite()->setOpacity(0.05f);
	}


	auto fileListInfo = getSprite<ds::ui::SmartScrollList>("specific_info");
	auto scrollBarInfo = getSprite<ds::ui::ScrollBar>("scroll_bar_info");
	if (fileListInfo && scrollBarInfo) {

		fileListInfo->setContentItemUpdatedCallback([this, fileList](ds::ui::SmartLayout* sl) {
			auto mediaIcon = sl->getSprite<ds::ui::Image>("media");
			if (!mediaIcon) return;
			auto cm = sl->getContentModel();
			auto theResource = cm.getPropertyResource("media_res");
			if (theResource.empty()) {
				mediaIcon->hide();
			} else {
				auto mediaType = theResource.getType();
				std::string thumbPath;

				if (mediaType == ds::Resource::IMAGE_TYPE) {
					thumbPath = "%APP%/data/images/ui/Camera.png";
				} else if (mediaType == ds::Resource::PDF_TYPE) {
					thumbPath = "%APP%/data/images/ui/PDF.png";
				} else if (mediaType == ds::Resource::VIDEO_TYPE) {
					thumbPath = "%APP%/data/images/ui/Movie.png";
				} else if (mediaType == ds::Resource::WEB_TYPE) {
					thumbPath = "%APP%/data/images/ui/Link.png";
				} else if (mediaType == ds::Resource::VIDEO_STREAM_TYPE) {
					thumbPath = "%APP%/data/images/ui/Source.png";
				} else {
					thumbPath = "%APP%/data/images/ui/Sparkles.png";
				}
				mediaIcon->show();
				mediaIcon->setImageFile(thumbPath, ds::ui::Image::IMG_CACHE_F);
			}

			sl->runLayout();
		});

		fileListInfo->setContentItemTappedCallback([this](ds::ui::SmartLayout* sl, ds::model::ContentModelRef theModel) {
			auto mpLayout = getSprite<ds::ui::LayoutSprite>("media_layout");
			auto theMP = getSprite<ds::ui::MediaPlayer>("the_media");
			if (!mpLayout || !theMP) return;

			std::string resourceString = "";
			auto theResource = theModel.getPropertyResource("media_res");
			if (theResource.empty()) {
				theMP->uninitialize();
			} else {
				theMP->setResource(theModel.getPropertyResource("media_res"));			

				resourceString.append("<b>Location:</b> (click to open) ");
				resourceString.append(theResource.getAbsoluteFilePath());
				resourceString.append("<br><b>Size:</b> ");
				resourceString.append(std::to_string((int)theResource.getWidth()) + " x " + std::to_string((int)theResource.getHeight()));
				resourceString.append("<br><b>Duration:</b> ");
				resourceString.append(std::to_string(theResource.getDuration()));
				resourceString.append("<br><b>Type:</b> ");
				resourceString.append(ds::utf8_from_wstr(theResource.getTypeName()));
				resourceString.append("<br><b>Thumbnail Id:</b> ");
				resourceString.append(std::to_string(theResource.getThumbnailId()));

				auto infoText = getSprite("resource_info");
				if(infoText){
					infoText->setTapCallback([this, theResource](ds::ui::Sprite* bs, const ci::vec3&) {
						if(theResource.getType() == ds::Resource::WEB_TYPE){
							std::string thecommand = "start " + theResource.getAbsoluteFilePath();
							system(thecommand.c_str());
						} else {
							Poco::Path mediaLocation = Poco::Path(theResource.getAbsoluteFilePath());
							//mediaLocation.makeParent();
							std::wstring wloc = ds::wstr_from_utf8("/select,\"" + mediaLocation.toString() + "\"");
							LPCWSTR loccy(wloc.c_str());
							std::wstring expl = ds::wstr_from_utf8("explorer.exe");
							LPCWSTR exply(expl.c_str());

							ShellExecute(0, NULL, exply, loccy, NULL, SW_SHOWNORMAL);
						}
					});
				}
			}

			setSpriteText("resource_info", resourceString);

			mpLayout->runLayout();
		});
		scrollBarInfo->linkScrollList(fileListInfo);
		ci::ColorA nubbinColor = mEngine.getColors().getColorFromName("ui_highlight");
		scrollBarInfo->getNubSprite()->setColor(nubbinColor);
		scrollBarInfo->getBackgroundSprite()->setOpacity(0.05f);

	}


	auto theRoot = mEngine.mContent.getChildByName("cms_root");
	if (theRoot.hasChildren()) {
		setData(theRoot);
		mInitialized = true;

		auto ni = getSprite("no_items");
		if (ni)ni->hide();
	}
}

void StoryController::setData(ds::model::ContentModelRef parent) {

	setContentModel(parent);
	auto fileList = getSprite<ds::ui::SmartScrollList>("scroll_list");
	if (fileList) {
		fileList->setContentList(parent);
	}

	std::vector<std::string> theNames;
	buildBreadCrumbs(theNames, parent);

	std::string crumbName;
	for (auto it : theNames) {
		crumbName.append(it);
		crumbName.append(" > ");
	}

	if (crumbName.size() > 3) {
		crumbName = crumbName.substr(0, crumbName.size() - 3);
	}

	setSpriteText("bread_crumbs", crumbName);
}

void StoryController::goBack() {
	auto theRoot = mEngine.mContent.getChildByName("cms_root");
	auto backItem = theRoot.getReference("valid_nodes", getContentModel().getPropertyInt("parent_id"));
	if (backItem.empty()) {
		setData(theRoot);
	} else {
		setData(backItem);
	}
}

void StoryController::buildBreadCrumbs(std::vector<std::string>& theNames, ds::model::ContentModelRef currentModel) {
	if (currentModel.empty()) return;
	auto theRoot = mEngine.mContent.getChildByName("cms_root");
	if (theRoot.getId() == currentModel.getId()) {
		theNames.emplace_back(theRoot.getPropertyString("name"));
		return;
	}

	auto theParent =currentModel.getPropertyInt("parent_id");
	if (theParent == theRoot.getId()) {
		buildBreadCrumbs(theNames, theRoot);
	} else  {
		buildBreadCrumbs(theNames, theRoot.getReference("valid_nodes", theParent));
	}
	theNames.emplace_back(currentModel.getPropertyString("name"));

}


void StoryController::showInfoPage(ds::model::ContentModelRef theInfo) {
	std::vector<ds::model::ContentModelRef> fakeModels;

	for (auto it : theInfo.getProperties()) {
		ds::model::ContentModelRef aModel;
		aModel.setProperty("property_name", it.first);
		aModel.setProperty("property_value", it.second.getString());
		if(!it.second.getResource().empty()){
			aModel.setPropertyResource("media_res", it.second.getResource());
		}
		fakeModels.emplace_back(aModel);
	}


	auto fileListInfo = getSprite<ds::ui::SmartScrollList>("specific_info");
	if(fileListInfo){
		fileListInfo->setContentList(fakeModels);
	}

	setSpriteText("info_title", theInfo.getPropertyString("name"));
	setSpriteText("info_kind", theInfo.getPropertyString("kind"));


	auto theMP = getSprite<ds::ui::MediaPlayer>("the_media");
	if (theMP) {
		theMP->uninitialize();
	}

	setSpriteText("resource_info", "");
}

} // namespace downstream

