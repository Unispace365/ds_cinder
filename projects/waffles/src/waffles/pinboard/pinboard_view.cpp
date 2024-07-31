#include "stdafx.h"

#include "pinboard_view.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/ui/scroll/scroll_area.h>

#include "app/app_defs.h"
#include "waffles/model/viewer_creation_args.h"
#include "waffles/waffles_events.h"
#include "waffles/common/ui_utils.h"

namespace waffles {
PinboardView::PinboardView(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/pinboard/pinboard.xml") {

	float theHeight = mEngine.getWafflesSettings().getFloat("pinboard:height", 0, mEngine.getWorldHeight() / 1.5f);
	int numRows = mEngine.getWafflesSettings().getInt("pinboard:num_rows", 0, 2);
	auto row_scr = getSprite<ds::ui::LayoutSprite>("rows_scroller");
	if (numRows < 1) numRows = 1;
	float floatRows = (float)numRows;
	float rowHeight = theHeight / floatRows - (floatRows - 1.0f) * mEngine.getWafflesSettings().getFloat("arrange:padding", 0, 20.0f);
	 
	if (row_scr) {
		for (int i = 0; i < numRows; i++) {
			auto newRow = new ds::ui::SmartLayout(mEngine, "waffles/pinboard/pinboard_row.xml");
			newRow->setSize(newRow->getWidth(), rowHeight);
			mRows.emplace_back(newRow);
			row_scr->addChildPtr(newRow);
		}
	}

	listenToEvents<CmsDataLoadCompleteEvent>([this](auto& e) {
		calculatePinboard();
	});

	listenToEvents<NoteSaveComplete>([this](auto& e) {
		for (auto it : mItems) {
			if (e.mNodeId == it->getContentModel().getId()) {
				it->getContentModel().setProperty("text", e.mTheNote);
				it->setSpriteText("the_note.primary_text", e.mTheNote);
				it->runLayout();
			}
		}
	});


	setContentUpdatedCallback([this] {
		calculatePinboard();
	});

	setSpriteClickFn("refresh_btn", [this] { 
		calculatePinboard(); 
	});

	setLayoutUpdatedFunction([this] {

		auto scrollr = getSprite<ds::ui::ScrollArea>("gallery_scroll");
		auto row_scr = getSprite<ds::ui::LayoutSprite>("rows_scroller");
		auto content = getSprite<ds::ui::LayoutSprite>("content_area");

		if (scrollr && row_scr  && content) {
			row_scr->runLayout();

			if (row_scr->getWidth() < content->getWidth()) {
				scrollr->setSize(row_scr->getWidth(), scrollr->getHeight());
				scrollr->enableScrolling(false);

			} else {
				scrollr->setSize(content->getWidth(), scrollr->getHeight());
				scrollr->enableScrolling(true);
				scrollr->recalculateSizes();
			}

			content->runLayout();
		}
	});
}

void PinboardView::calculatePinboard() {

	auto actualViewers = mItems;
	mItems.clear();
	ds::model::ContentModelRef thePinboard = mEngine.mContent.getChildByName("current_pinboard");
	auto theList = thePinboard.getPropertyListInt("pinboard_items_node_ids");

	std::vector<ds::ui::SmartLayout*> viewersToRemove;
	std::vector<int> nodesToLaunch  = theList;
	std::vector<ds::model::ContentModelRef> nodeModelsToLaunch;


	for (auto it : nodesToLaunch) {
		auto theNode = mEngine.mContent.getChildByName("cms_root").getReference("valid_nodes", it);

		// if this is a "note" node, it'll be under an event and not in the root
		if (theNode.empty()) {
			theNode = mEngine.mContent.getChildByName("current_events").getDescendant("waffles_nodes", it);
		}

		if (theNode.empty()) {
			DS_LOG_WARNING("Invalid pinboard item for id=" << it);
			continue;
		}

		nodeModelsToLaunch.emplace_back(theNode);
	}

	// if the viewer exists in the pinboard item list, we're good and continue
	// if the viewer doesn't exist in the pinboard item list, add the viewer to the "to remove" list
	// if there are extra items in the pinboard list, launch them
	for (auto it : actualViewers) {
		int nodeId = it->getContentModel().getId();

		bool viewerExists = false;

		for(auto tit = nodeModelsToLaunch.begin(); tit < nodeModelsToLaunch.end(); tit++){
			if(nodeId == (*tit).getId()){
				nodeModelsToLaunch.erase(tit);
				viewerExists = true;
				mItems.emplace_back(it);
				break;
			}
		}

		if (viewerExists) {
			continue;
		}

		viewersToRemove.emplace_back(it);
	}

	for (auto it : viewersToRemove) {
		it->release();
	}

	auto scrollr = getSprite<ds::ui::ScrollArea>("gallery_scroll");
	auto row_scr = getSprite<ds::ui::LayoutSprite>("rows_scroller");

	if(scrollr && row_scr  && !mRows.empty()){

		for (auto theNode : nodeModelsToLaunch) {

			ds::ui::SmartLayout* shortestRow = nullptr;
			float shortestWidth = 1000000.0f;
			for (auto it : mRows) {
				float thisRowW = it->getWidth();
				if (thisRowW < shortestWidth) {
					shortestRow = it;
					shortestWidth = thisRowW;
				}
			}

			if (!shortestRow) shortestRow = mRows.front();
			float rowHeight = shortestRow->getHeight();

			auto newItem = new ds::ui::SmartLayout(mEngine, "waffles/pinboard/pinboard_item.xml");

			newItem->setContentModel(theNode);

			if(theNode.getPropertyString("kind") == "note"){
				auto noteRoot = newItem->getSprite("the_note.root_layout");
				if(noteRoot){
					noteRoot->setSize(rowHeight * 0.8f, rowHeight);
				}

				/* auto accentColor = getRandomAccentColor(mEngine);
				if (auto noteBackground = newItem->getSprite("the_note.background")) {
					noteBackground->setColor(accentColor);
				} */

				newItem->setSpriteClickFn("the_note.edit_note.the_button", [this, newItem] {
					auto updatedMedia = mEngine.mContent.getChildByName("cms_root").getReference("valid_nodes", newItem->getContentModel().getId());
					if (updatedMedia.empty()) updatedMedia = newItem->getContentModel();
					mEngine.getNotifier().notify(RequestViewerLaunchEvent(ViewerCreationArgs(updatedMedia, VIEW_TYPE_NOTE_TAKING, getGlobalCenterPosition())));
				});

				newItem->runLayout();
			}

			mItems.emplace_back(newItem);

			newItem->setTapCallback([this, newItem](ds::ui::Sprite* bs, const ci::vec3& pos) {
				mEngine.getNotifier().notify(RequestViewerLaunchEvent(ViewerCreationArgs(newItem->getContentModel(), VIEW_TYPE_TITLED_MEDIA_VIEWER, pos)));
			});
			newItem->setProcessTouchCallback([this, scrollr](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
				if (ti.mPhase == ds::ui::TouchInfo::Moved && glm::distance(ti.mCurrentGlobalPoint, ti.mStartPoint) > mEngine.getMinTapDistance()) {
					bs->passTouchToSprite(scrollr->getSpriteToPassTo(), ti);
					return;
				}
			});

			shortestRow->addChildPtr(newItem);
			shortestRow->runLayout();

			newItem->tweenAnimateOn(true, mEngine.getAnimDur(), 0.0f);

		}

		runLayout();
	}

	auto noItemsMessage = getSprite("no_items");
	if (noItemsMessage) {
		if (mItems.empty()) {
			noItemsMessage->show();
			noItemsMessage->tweenOpacity(1.0f, mEngine.getAnimDur());
		} else {
			noItemsMessage->tweenOpacity(0.0f, mEngine.getAnimDur(), 0.0f, ci::easeNone, [noItemsMessage] { noItemsMessage->hide(); });
		}
	}
}

void PinboardView::refreshContent() {
	calculatePinboard();
}

}

