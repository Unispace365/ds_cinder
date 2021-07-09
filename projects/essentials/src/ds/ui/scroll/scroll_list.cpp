#include "stdafx.h"

#include "scroll_list.h"

#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/debug/logger.h>

namespace ds{
namespace ui{

ScrollList::ScrollList(ds::ui::SpriteEngine& engine, const bool vertical)
	: ds::ui::Sprite(engine)
	, mScrollArea(nullptr)
	, mStartPositionX(10.0f)
	, mStartPositionY(0.0f)
	, mIncrementAmount(50.0f)
	, mAnimateOnDeltaDelay(0.0f)
	, mAnimateOnStartDelay(0.0f)
	, mVerticalScrolling(vertical)
	, mFillFromTop(true)
	, mGridLayout(false)
	, mSpecialLayout(false)
	, mVaryingSizeLayout(false)
	, mTargetRow(0)
	, mTargetColumn(0)
	, mMatrixPadding(0)
{
	mScrollArea = new ds::ui::ScrollArea(mEngine, getWidth(), getHeight(), mVerticalScrolling);
	if(mScrollArea){
		mScrollArea->setScrollUpdatedCallback([this](ds::ui::Sprite*){
			assignItems();
			if(mScrollUpdatedCallback){
				mScrollUpdatedCallback();
			}
		});
		mScrollArea->setScrollerTouchedCallback([this]{
			mLastUpdateTime = Poco::Timestamp().epochMicroseconds();
		});
		mScrollArea->setFadeColors(ci::ColorA(0.0f, 0.0f, 0.0f, 1.0f), ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));
		mScrollArea->setFadeHeight(50.0f);
		mScrollArea->setUseFades(true);
		addChildPtr(mScrollArea);
	}

	mScrollableHolder = new ds::ui::Sprite(mEngine);


	if(mScrollableHolder){
		mScrollArea->addSpriteToScroll(mScrollableHolder);
		mScrollableHolder->enable(false);
	}

	mLastUpdateTime = Poco::Timestamp().epochMicroseconds();

	enable(false);
}

void ScrollList::setContent(const std::vector<int>& models){
	clearItems();

	for(auto it = models.begin(); it < models.end(); ++it){
		auto placeholder = ItemPlaceHolder((*it));
		placeholder.mSize = ci::vec2(mIncrementAmount);
		mItemPlaceHolders.push_back(placeholder);
	}

	layout();
	if(mScrollArea){
		mScrollArea->resetScrollerPosition();
	}
	animateItemsOn();
}

void ScrollList::animateItemsOn(){
	float theDelay = mAnimateOnStartDelay;
	for(auto it = mItemPlaceHolders.begin(), it2 = mItemPlaceHolders.end(); it != it2; ++it){
		if((*it).mAssociatedSprite){
			if(mAnimateOnCallback) mAnimateOnCallback((*it).mAssociatedSprite, theDelay);
			theDelay += mAnimateOnDeltaDelay;
		}
	}
}

void ScrollList::layout(){

	layoutItems();


	if(mVerticalScrolling){
		float scrollyHeight = mScrollableHolder->getHeight();

		if(scrollyHeight < getHeight()){
			scrollyHeight = getHeight();
		}

		if(mScrollableHolder){
			mScrollableHolder->setSize(getWidth(), scrollyHeight);
		}
		if (getPerspective()){
			if(mFillFromTop){
				pushItemsTop();
			}
		} else if(!mFillFromTop){
			pushItemsTop();
		}

	} else {
		float scrollyWidth = mScrollableHolder->getWidth();

		if(scrollyWidth < getWidth()){
			scrollyWidth = getWidth();
		}

		if(mScrollableHolder){
			mScrollableHolder->setSize(scrollyWidth, getHeight());
		}
	}

	if(mScrollArea){
		mScrollArea->setScrollSize(getWidth(), getHeight());
	}


	assignItems();
}

void ScrollList::pushItemsTop(){
	if (mVerticalScrolling){
		if (mItemPlaceHolders.empty()) return;
		float scrollHeight = mScrollableHolder->getHeight();
		if(getPerspective()){
			const auto incrementAmount = mVaryingSizeLayout ? mItemPlaceHolders[0].mSize.y : mIncrementAmount;
			if(mItemPlaceHolders[0].mY < scrollHeight - mStartPositionY - incrementAmount){
				float delta = scrollHeight - mItemPlaceHolders[0].mY - mStartPositionY - incrementAmount;
				for(auto it = mItemPlaceHolders.begin(); it < mItemPlaceHolders.end(); ++it){
					(*it).mY += delta;
				}
			}
		} else {
			const auto incrementAmount = mVaryingSizeLayout ? mItemPlaceHolders.back().mSize.y : mIncrementAmount;
			if(mItemPlaceHolders.back().mY < scrollHeight - incrementAmount){
				float delta = scrollHeight - mItemPlaceHolders.back().mY - incrementAmount;
				for(auto it = mItemPlaceHolders.begin(); it < mItemPlaceHolders.end(); ++it){
					(*it).mY += delta;
				}
			}
		}
	}
}

void ScrollList::setGridLayout(const bool doGrid, const ci::vec2& gridIncrement){
	mGridLayout = doGrid;
	mGridIncrement = gridIncrement;

	layout();
}

void ScrollList::layoutItemsGrid(){
	float xp = mStartPositionX;
	float yp = mStartPositionY;
	// TODO: handle perspective
	//const bool isPerspective = Sprite::getPerspective();
	float wrapWidth = getWidth();
	bool justwrapped = false;
	for(auto it = mItemPlaceHolders.begin(); it < mItemPlaceHolders.end(); ++it){
		(*it).mX = xp;
		(*it).mY = yp;

		if(mVerticalScrolling) {
			xp += mGridIncrement.x;
			if(xp > getWidth() - mGridIncrement.x) {
				xp = mStartPositionX;
				yp += mGridIncrement.y;
				justwrapped = true;
			} else {
				justwrapped = false;
			}
		} else {
			yp += mGridIncrement.y;
			if(yp > getHeight() - mGridIncrement.y) {
				yp = mStartPositionY;
				xp += mGridIncrement.x;
				justwrapped = true;
			} else {
				justwrapped = false;
			}
		}
	}

	if(!justwrapped){
		if(mVerticalScrolling) {
			yp += mGridIncrement.y;
		} else {
			xp += mGridIncrement.x;
		}
	}
	if(mScrollableHolder){
		if(mVerticalScrolling) {
			mScrollableHolder->setSize(getWidth(), yp);
		} else {
			mScrollableHolder->setSize(xp, getHeight());
		}
	}
}

void ScrollList::setMatrixLayout(const bool doSpcial, const int targetRow, const int targetColumn, const float gapping)
{
	mSpecialLayout = doSpcial;
	mTargetRow = targetRow;
	mTargetColumn = targetColumn;
	mMatrixPadding = gapping;
	layout();
}

void ScrollList::layoutItemsMatrix()
{
	float xp = mStartPositionX;
	float yp = mStartPositionY;
	const bool isPerspective = Sprite::getPerspective();
	float totalHeight = yp;
	if (mVerticalScrolling){
		if (mTargetColumn == 0)	return;
		totalHeight = std::ceil((float)mItemPlaceHolders.size() / mTargetColumn) * mIncrementAmount + mStartPositionY * 2.0f;
		if (isPerspective) yp = totalHeight - mIncrementAmount - mStartPositionY;
	}
	else
	{
		if (mTargetRow == 0)	return;
	}
	int index = 1;
	for(auto it = mItemPlaceHolders.begin(); it < mItemPlaceHolders.end(); ++it) {
		(*it).mX = xp;
		(*it).mY = yp;

		if(mVerticalScrolling) {
			int count = index % mTargetColumn;
			if(count == 0) {
				if(isPerspective) {
					yp -= mIncrementAmount;
				} else {
					yp += mIncrementAmount;
				}
				xp = mStartPositionX;
			} else {
				xp += mMatrixPadding;
			}

		} else {
			int count = index % mTargetRow;
			if(count == 0) {
				xp += mIncrementAmount;
				yp = mStartPositionY;
			} else {
				if(index == mItemPlaceHolders.size()) {
					xp += mIncrementAmount;
				}
				yp += mMatrixPadding;
			}
		}
		index++;
	}

	if (mVerticalScrolling){
		mScrollableHolder->setSize(getWidth(), totalHeight);
	}
	else{
		xp += mStartPositionX;
		mScrollableHolder->setSize(xp, getHeight());
	}
}

// Override if you need to do something special with the layout, otherwise just set start positions and increment amounts
void ScrollList::layoutItems(){
	if(mGridLayout){
		layoutItemsGrid();
		return;
	}
	if (mSpecialLayout)
	{
		layoutItemsMatrix();
		return;
	}

	float xp = mStartPositionX;
	float yp = mStartPositionY;
	const bool isPerspective = Sprite::getPerspective();
	float totalHeight = yp;

	if (mVerticalScrolling){
		if (mVaryingSizeLayout) {
			totalHeight += mStartPositionY * 2.f;
			for (auto item : mItemPlaceHolders) {
				totalHeight += item.mSize.y;
			}
		} else {
			totalHeight = (float)(mItemPlaceHolders.size()) * mIncrementAmount + mStartPositionY * 2.0f;
		}
		if(isPerspective) yp = totalHeight - mIncrementAmount - mStartPositionY;
	}

	if (mVaryingSizeLayout) {
		for (auto it = mItemPlaceHolders.begin(); it < mItemPlaceHolders.end(); ++it) {
			(*it).mX = xp;
			(*it).mY = yp;

			if (mVerticalScrolling) {
				if (isPerspective) {
					yp -= (*it).mSize.y;
				} else {
					yp += (*it).mSize.y;
				}
			} else {
				xp += (*it).mSize.x;
			}
		}
	} else {
		for (auto it = mItemPlaceHolders.begin(); it < mItemPlaceHolders.end(); ++it) {
			(*it).mX = xp;
			(*it).mY = yp;

			if (mVerticalScrolling) {
				if (isPerspective) {
					yp -= mIncrementAmount;
				} else {
					yp += mIncrementAmount;
				}
			} else {
				xp += mIncrementAmount;
			}
		}
	}


	if(mVerticalScrolling){
		mScrollableHolder->setSize(getWidth(), totalHeight);
	} else {
		xp += mStartPositionX;
		mScrollableHolder->setSize(xp, getHeight());
	}
}


void ScrollList::clearItems(){
	for(auto it = mItemPlaceHolders.begin(), it2 = mItemPlaceHolders.end(); it != it2; ++it){
		if(it->mAssociatedSprite){
			it->mAssociatedSprite->hide();
			mReserveItems.push_back(it->mAssociatedSprite);
		}
	}

	mItemPlaceHolders.clear();

	if(mScrollArea){
		mScrollArea->setScrollSize(mScrollArea->getWidth(), mScrollArea->getHeight());
	}
}

void ScrollList::assignItems(){
	if(!mScrollArea || !mScrollableHolder) return;

	ci::vec2 scrollPos = mScrollArea->getScrollerPosition();
	float scrollHeight = mScrollArea->getHeight();
	float scrollWidth = mScrollArea->getWidth();

	if (mVaryingSizeLayout) {
		float incrementedPosition = mVerticalScrolling ? mStartPositionY : mStartPositionX;
		for (auto& it : mItemPlaceHolders) {
			if (mVerticalScrolling) it.mY = incrementedPosition;
			else it.mX = incrementedPosition;

			float itemY = scrollPos.y + it.mY;
			float itemX = scrollPos.x + it.mX;

			const auto incrementAmount = mVaryingSizeLayout ? (mVerticalScrolling ? it.mSize.y : it.mSize.x) : mIncrementAmount;

			if ((mVerticalScrolling && ((itemY + incrementAmount > 0.0f) && (itemY < scrollHeight))) ||
				(!mVerticalScrolling && ((itemX + incrementAmount > 0.0f) && (itemX < scrollWidth)))
				) {
				if (it.mAssociatedSprite) {
					it.mAssociatedSprite->setPosition(it.mX, it.mY);
				} else {
					/// create sprite
					ds::ui::Sprite* sprite = nullptr;
					if (!mReserveItems.empty()) {
						sprite = mReserveItems.back();
						mReserveItems.pop_back();
					} else {
						if (mCreateItemCallback) sprite = mCreateItemCallback();
						if (sprite) {
							sprite->setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti) { handleItemTouchInfo(sp, ti); });
							sprite->setTapCallback([this, sprite](ds::ui::Sprite* bs, const ci::vec3 cent) {
								Poco::Timestamp::TimeVal nowwwy = Poco::Timestamp().epochMicroseconds();
								float timeDif = (float)(nowwwy - mLastUpdateTime) / 1000000.0f;
								if (timeDif < 0.2f) {
									DS_LOG_VERBOSE(2, "Too soon since the last touch to tap this list!");
									return;
								}
								mLastUpdateTime = Poco::Timestamp().epochMicroseconds();
								if (mItemTappedCallback) mItemTappedCallback(sprite, cent);
							});
							mScrollableHolder->addChildPtr(sprite);
						} else {
							DS_LOG_WARNING("Didn't create a sprite for scroll list! Use the callback and make sprites when we need them!!");
						}
					}

					if (sprite) {
						if (mSetDataCallback) mSetDataCallback(sprite, it.mDbId);
						sprite->setPosition(it.mX, it.mY);
						ci::vec2 spSize = ci::vec2(sprite->getSize());
						if (it.mSize != spSize) {
							it.mSize = spSize;
						}

						sprite->show();
						it.mAssociatedSprite = sprite;
					}
				}
			} else {
				// item is offscreen, put any associated sprites in reserve
				if (it.mAssociatedSprite) {
					mReserveItems.push_back(it.mAssociatedSprite);
					it.mAssociatedSprite = nullptr;
				}
			}

			if (mVerticalScrolling) incrementedPosition = it.mY + it.mSize.y;
			else incrementedPosition = it.mX + it.mSize.x;
		}

		if (mVerticalScrolling) {
			if (mScrollableHolder->getHeight() != incrementedPosition) {
				mScrollableHolder->setSize(getWidth(), incrementedPosition);
				if (mScrollArea) mScrollArea->recalculateSizes();
			}
		} else if (mScrollableHolder->getWidth() != incrementedPosition) {
			mScrollableHolder->setSize(incrementedPosition, getHeight());
			if (mScrollArea) mScrollArea->recalculateSizes();
		}
	} else {
		std::vector<int> needsSprite;

		// Look through all the placeholdser and find all the items that should be onscreen
		// push the offscreen sprites into the reserve vector
		for (auto it = mItemPlaceHolders.begin(), it2 = mItemPlaceHolders.end(); it != it2; ++it) {
			float itemY = scrollPos.y + it->mY;
			float itemX = scrollPos.x + it->mX;

			const auto incrementAmount = mVaryingSizeLayout ? (mVerticalScrolling ? it->mSize.y : it->mSize.x) : mIncrementAmount;

			if ((mVerticalScrolling && ((itemY + incrementAmount > 0.0f) && (itemY < scrollHeight))) ||
				(!mVerticalScrolling && ((itemX + incrementAmount > 0.0f) && (itemX < scrollWidth)))
				) {
				if (it->mAssociatedSprite) {
					it->mAssociatedSprite->setPosition(it->mX, it->mY);
				} else {
					needsSprite.push_back(static_cast<int>(it - mItemPlaceHolders.begin()));
				}
			} else {
				if (it->mAssociatedSprite) {
					mReserveItems.push_back(it->mAssociatedSprite);
				}
				it->mAssociatedSprite = nullptr;
			}
		}

		// give all the placeholders that need a sprite
		for (auto it = needsSprite.begin(), it2 = needsSprite.end(); it != it2; ++it) {
			ds::ui::Sprite* sprite = nullptr;
			if (!mReserveItems.empty()) {
				sprite = mReserveItems.back();
				mReserveItems.pop_back();
			} else {
				if (mCreateItemCallback) sprite = mCreateItemCallback();
				if (sprite) {
					sprite->setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti) { handleItemTouchInfo(sp, ti); });
					sprite->setTapCallback([this, sprite](ds::ui::Sprite* bs, const ci::vec3 cent) {
						Poco::Timestamp::TimeVal nowwwy = Poco::Timestamp().epochMicroseconds();
						float timeDif = (float)(nowwwy - mLastUpdateTime) / 1000000.0f;
						if (timeDif < 0.2f) {
							DS_LOG_VERBOSE(2, "Too soon since the last touch to tap this list!");
							return;
						}
						mLastUpdateTime = Poco::Timestamp().epochMicroseconds();
						if (mItemTappedCallback) mItemTappedCallback(sprite, cent);
					});
					mScrollableHolder->addChildPtr(sprite);
				} else {
					DS_LOG_WARNING("Didn't create a sprite for scroll list! Use the callback and make sprites when we need them!!");
				}
			}

			auto &placeHolder = mItemPlaceHolders[*it];

			if (sprite) {

				if (mSetDataCallback) mSetDataCallback(sprite, placeHolder.mDbId);
				sprite->setPosition(placeHolder.mX, placeHolder.mY);
				sprite->show();
				placeHolder.mAssociatedSprite = sprite;
			}
		}
	}

	// hide any extras
	for(auto it = mReserveItems.begin(), it2 = mReserveItems.end(); it != it2; ++it){
		auto sprite = *it;
		sprite->hide();
	}

}

void ScrollList::handleItemTouchInfo(ds::ui::Sprite* bs, const TouchInfo& ti){
	if(bs){
		if(mStateChangeCallback) mStateChangeCallback(bs, ti.mNumberFingers > 0);

		if(mScrollArea  && ti.mPhase == ds::ui::TouchInfo::Moved &&ci::distance(ti.mCurrentGlobalPoint,ti.mStartPoint) > mEngine.getMinTapDistance()){
			if(mStateChangeCallback) mStateChangeCallback(bs, false);
			bs->passTouchToSprite(mScrollArea->getSpriteToPassTo(), ti);
			return;
		}
	}
}

void ScrollList::onSizeChanged(){
	layout();
}

void ScrollList::setItemTappedCallback(const std::function<void(ds::ui::Sprite*, const ci::vec3& cent)> &func){
	mItemTappedCallback = func;
}

void ScrollList::setCreateItemCallback(const std::function<ds::ui::Sprite*() > &func){
	mCreateItemCallback = func;
}

void ScrollList::setDataCallback(const std::function<void(ds::ui::Sprite*, const int dbId) > &func){
	mSetDataCallback = func;
}

void ScrollList::setAnimateOnCallback(const std::function<void(ds::ui::Sprite*, const float delay)>&func){
	mAnimateOnCallback = func;
}

void ScrollList::setStateChangeCallback(const std::function<void(ds::ui::Sprite*, const bool highlighted)>&func){
	mStateChangeCallback = func;
}

void ScrollList::setScrollUpdatedCallback(const std::function<void(void)> &func){
	mScrollUpdatedCallback = func;
}

void ScrollList::setLayoutParams(const float startPositionX, const float startPositionY, const float incremenetAmount, const bool fill_from_top){
	mStartPositionX = startPositionX;
	mStartPositionY = startPositionY;
	mIncrementAmount = incremenetAmount;
	mFillFromTop = fill_from_top;
}

void ScrollList::setAnimateOnParams(const float startDelay, const float deltaDelay){
	mAnimateOnStartDelay = startDelay;
	mAnimateOnDeltaDelay = deltaDelay;
}

void ScrollList::forEachLoadedSprite(std::function<void(ds::ui::Sprite*)> func){
	if(!func) return;
	for(auto it = mItemPlaceHolders.begin(); it < mItemPlaceHolders.end(); ++it){
		if((*it).mAssociatedSprite){
			func((*it).mAssociatedSprite);
		}
	}

	for(auto it = mReserveItems.begin(); it < mReserveItems.end(); ++it){
		func((*it));
	}
}

}
}
