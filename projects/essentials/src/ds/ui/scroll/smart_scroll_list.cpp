#include "stdafx.h"

#include "smart_scroll_list.h"

#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/scroll/scroll_area.h>

namespace ds::ui {

SmartScrollList::SmartScrollList(ds::ui::SpriteEngine& eng, const std::string& itemLayoutFile, const bool vertical)
  : ScrollList(eng, vertical)
  , mContentItemTapped(nullptr) {

	if (!itemLayoutFile.empty()) {
		setItemLayoutFile(itemLayoutFile);
	}

	setItemTappedCallback([this](ds::ui::Sprite* bs, const ci::vec3& cent) {
		SmartLayout* rpi = dynamic_cast<SmartLayout*>(bs);
		if (rpi) {
			if (mContentItemTapped) {
				mContentItemTapped(rpi, rpi->getContentModel());
			}
		}
	});

	setDataCallback([this](ds::ui::Sprite* bs, int dbId) {
		SmartLayout* rpi = dynamic_cast<SmartLayout*>(bs);
		if (rpi) {
			rpi->setContentModel(mContentMap[dbId]);

			if (mContentItemUpdated) {
				mContentItemUpdated(rpi);
			}
		}
	});

	setAnimateOnCallback([](ds::ui::Sprite* bs, const float delay) {
		SmartLayout* rpi = dynamic_cast<SmartLayout*>(bs);
		if (rpi) {
			/// todo?
		}
	});

	setStateChangeCallback([](ds::ui::Sprite* bs, const bool highlighted) {
		SmartLayout* rpi = dynamic_cast<SmartLayout*>(bs);
		if (rpi) {
			auto highlightSprite = rpi->getSprite("highlight");
			if (highlightSprite) {
				if (highlighted) {
					highlightSprite->show();
				} else {
					highlightSprite->hide();
				}
			}
		}
	});
}

void SmartScrollList::setContentList(ds::model::ContentModelRef parentModel) {
	setContentList(parentModel.getChildren());
}

void SmartScrollList::setContentList(std::vector<ds::model::ContentModelRef> theContents) {
	mContentMap.clear();
	int itemId = 1;

	std::vector<int> productIds;
	for (auto it : theContents) {
		productIds.emplace_back(itemId);
		mContentMap[itemId] = it;
		itemId++;
	}
	setContent(productIds);
}


void SmartScrollList::setContentListMaintainPosition(std::vector<ds::model::ContentModelRef> theContents) {
	if (!getScrollArea()) return;
	auto prePercent = getScrollArea()->getScrollPercent();
	setContentList(theContents);
	getScrollArea()->setScrollPercent(prePercent);
}

void SmartScrollList::setContentListMaintainPosition(ds::model::ContentModelRef parentModel) {
	setContentListMaintainPosition(parentModel.getChildren());
}

void SmartScrollList::setItemLayoutFile(const std::string& itemLayout) {
	if (itemLayout.empty()) {
		DS_LOG_WARNING("Can't set a blank layout for sub items in SmartScrollList");
		return;
	}

	/// grab the height from the item, then get rid of it
	ds::ui::SmartLayout* tempItem = new ds::ui::SmartLayout(mEngine, itemLayout);
	tempItem->runLayout();
	auto itemSize = tempItem->getSize();
	tempItem->release();

	if (isVerticalScroll()) {
		setLayoutParams(0.0f, 0.0f, itemSize.y);
	} else {
		setLayoutParams(0.0f, 0.0f, itemSize.x);
	}

	// If the layout file might have changed, any reserve or current sprites could be invalid.
	clearItems();
	for (auto res : mReserveItems) {
		if (res) res->release();
	}
	mReserveItems.clear();

	setCreateItemCallback([this, itemLayout]() -> ds::ui::Sprite* { return new SmartLayout(mEngine, itemLayout); });
}

void SmartScrollList::layoutItems() {
	if (mVaryingSizeLayout) {
		float xp		  = mStartPositionX;
		float yp		  = mStartPositionY;
		float totalHeight = yp + mStartPositionY * 2.f;
		for (auto& item : mItemPlaceHolders) {
			item.mX = xp;
			item.mY = yp;

			if (item.mAssociatedSprite) {
				item.mSize = ci::vec2(item.mAssociatedSprite->getSize());
			}

			totalHeight += item.mSize.y;
			yp += item.mSize.y;
		}

		mScrollableHolder->setSize(getWidth(), totalHeight);
	} else {
		ScrollList::layoutItems();
	}
}

} // namespace ds::ui
