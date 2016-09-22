#include "infinity_scroll_list.h"

#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <algorithm>

namespace ds{
	namespace ui{

		infinityList::infinityList(SpriteEngine& engine, const float startWidth, const float startHeight, const bool vertical /*= true*/)\
			: Sprite(engine)
			, mSpriteMomentum(mEngine)
			, mVertical(vertical)
			, mScroller(nullptr)
			, mPadding(0.0f)
			, mStartPositionX(10.0f)
			, mStartPositionY(0.0f)
			, mIncrementAmount(350.0f)
			, mOnScreenItemSize(0)
			, mTopIndex(-1)
			, mBottomIndex(-1)
			, mIsOnTweenAnimation(false)
			, mTweenAnimationDuration(0.4f)
			, mTweenAnimationDelay(0.0f)
			, mTweenAnimationEaseFn(ci::EaseNone())
			, mIsTurnOnStepSwipe(false)

		{
			setSize(startWidth, startHeight);
			mSpriteMomentum.setMass(8.0f);
			mSpriteMomentum.setFriction(0.5f);
			mSpriteMomentum.setMomentumParent(this);

			enable(false);
			setClipping(true);

			mScroller = new Sprite(mEngine);
			if (mScroller){
				mScroller->mExportWithXml = false;
				mScroller->setSize(startWidth, startHeight);
				mScroller->enable(true);
				mScroller->enableMultiTouch(MULTITOUCH_INFO_ONLY);
				mScroller->setProcessTouchCallback([this](Sprite* bs, const TouchInfo& ti){
					handleScrollTouch(bs, ti);
					mLastUpdateTime = Poco::Timestamp().epochMicroseconds();
				});
				mSpriteMomentum.setMomentumParent(mScroller);
				addChildPtr(mScroller);
			}

			mLastUpdateTime = Poco::Timestamp().epochMicroseconds();
		}

		void infinityList::setContent(const std::vector<int>& dbIds)
		{
			clearItems();
			initFillScreen();

			mTopIndex = dbIds.size() - 1;

			mItemPlaceHolders.push_back(ItemPlaceHolder(dbIds[mTopIndex]));
			if (dbIds.size() > 1)
			{
				for (auto it = 0; it < dbIds.size() - 1; ++it){
					mItemPlaceHolders.push_back(ItemPlaceHolder(dbIds[it]));
				}
			}


			layout();
		}

		void infinityList::setItemTappedCallback(const std::function<void(Sprite*, const ci::Vec3f& cent)> &func){
			mItemTappedCallback = func;
		}

		void infinityList::setCreateItemCallback(const std::function<Sprite*() > &func){
			mCreateItemCallback = func;
		}

		void infinityList::setDataCallback(const std::function<void(Sprite*, const int dbId) > &func){
			mSetDataCallback = func;
		}

		void infinityList::setAnimateOnCallback(const std::function<void(Sprite*, const float delay)>&func){
			mAnimateOnCallback = func;
		}

		void infinityList::setStateChangeCallback(const std::function<void(Sprite*, const bool highlighted)>&func){
			mStateChangeCallback = func;
		}

		void infinityList::setScrollUpdatedCallback(const std::function<void(void)> &func){
			mScrollUpdatedCallback = func;
		}

		void infinityList::setLayoutParams(const float startPositionX, const float startPositionY, const float incremenetAmount)
		{
			mStartPositionX = startPositionX;
			mStartPositionY = startPositionY;
			mIncrementAmount = incremenetAmount;
		}

		void infinityList::setTweenAnimationParams(const float duration, const float delay /*= 0.0f*/, const ci::EaseFn fn/*= ci::EaseNone()*/)
		{
			mTweenAnimationDuration = duration;
			mTweenAnimationDelay = delay;
			mTweenAnimationEaseFn = fn;
		}

		void infinityList::nextItem()
		{
			if (mOnScreenItemSize < 2)
				return;

			auto pos = mOnScreenItemList[1].mAssociatedSprite->getPosition();
			float targetAmount = 0.0f;
			if (mVertical)
			{

			}
			else
			{
				targetAmount = mStartPositionX - mIncrementAmount - pos.x;
			}
			tweenItemPos(targetAmount);
		}

		void infinityList::previousItem()
		{
			if (mOnScreenItemSize < 1)
				return;

			auto pos = mOnScreenItemList[0].mAssociatedSprite->getPosition();
			float targetAmount = 0.0f;
			if (mVertical)
			{

			}
			else
			{
				targetAmount = mStartPositionX - pos.x;
			}
			tweenItemPos(mIncrementAmount);
		}

		void infinityList::turnOnStepSwipe()
		{
			if (!mIsTurnOnStepSwipe)
			{
				mIsTurnOnStepSwipe = true;
			}
		}

		void infinityList::updateServer(const ds::UpdateParams& p)
		{
			Sprite::updateServer(p);
		}

		void infinityList::clearItems()
		{
			for (auto it = mItemPlaceHolders.begin(), it2 = mItemPlaceHolders.end(); it != it2; ++it){
				if (it->mAssociatedSprite){
					it->mAssociatedSprite->release();
				}
			}

			mItemPlaceHolders.clear();
		}

		void infinityList::assignItems()
		{
			float y = mStartPositionY;
			float x = mStartPositionX;
			float scrollHeight = mScroller->getHeight();
			float scrollWidth = mScroller->getWidth();

			float xp = mStartPositionX;
			float yp = mStartPositionY;
			if (mVertical){
				yp -= mIncrementAmount;
			}
			else
			{
				xp -= mIncrementAmount;
			}

			mOnScreenItemList.clear();
			for (auto i = 0; i < mOnScreenItemSize; ++i){

				mBottomIndex++;

				if (mBottomIndex == mItemPlaceHolders.size())
					mBottomIndex = 0;
				auto &placeHolder = mItemPlaceHolders[mBottomIndex];


				createSprite(placeHolder);

				placeHolder.mAssociatedSprite->setPosition(xp, yp);

				if (mVertical){
					yp += mIncrementAmount;
				}
				else
				{
					xp += mIncrementAmount;
				}

				mOnScreenItemList.push_back(placeHolder);
			}
		}

		void infinityList::initItemStart(int itemNum){
			mBottomIndex = itemNum % (mItemPlaceHolders.size() - 1) - 1;
			assignItems();
		}

		void infinityList::handleScrollTouch(Sprite* bs, const TouchInfo& ti)
		{
			if(mSwipeCallback) mSwipeCallback(bs, ti.mCurrentGlobalPoint);
			if (ti.mPhase == TouchInfo::Added){
				mSpriteMomentum.deactivate();
			}
			else if (ti.mPhase == TouchInfo::Removed && ti.mNumberFingers == 0){
				mSpriteMomentum.activate();
			}
			else if (ti.mPhase == TouchInfo::Moved && ti.mNumberFingers > 0){
				auto deltaPoint = ti.mDeltaPoint;

				if (mScroller){
					if (mIsTurnOnStepSwipe)
					{
						if (ti.mCurrentGlobalPoint.distance(ti.mStartPoint) > mEngine.getMinTapDistance())
						{
							if (mVertical)
							{

							}
							else
							{
								if (ti.mDeltaPoint.x > 0)
									previousItem();
								else
									nextItem();
							}
						}
					}
					else
					{
						if (mVertical){
							float yDelta = deltaPoint.y / ti.mNumberFingers;
							if (getPerspective()){
								yDelta = -yDelta;
							}
							itemPosUpdated(yDelta);
						}
						else {
							float xDelta = deltaPoint.x / ti.mNumberFingers;
							itemPosUpdated(xDelta);
						}
					}
				}
			}

		}

		void infinityList::itemPosUpdated(const float delta)
		{
			if (mOnScreenItemList.empty())
				return;

			for (auto it = mOnScreenItemList.begin(); it < mOnScreenItemList.end(); it++)
			{
				auto targetSprite = (*it).mAssociatedSprite;
				if (mVertical)
				{
					targetSprite->move(0.0f, delta);
					if (targetSprite->getPosition().y <= -mIncrementAmount + mStartPositionY || targetSprite->getPosition().y >= mScroller->getHeight())
					{
						(*it).mOnscrren = false;
					}
					else
					{
						(*it).mOnscrren = true;
					}
				}
				else
				{
					targetSprite->move(delta, 0.0f);
					if (targetSprite->getPosition().x <= -mIncrementAmount + mStartPositionX || targetSprite->getPosition().x >= mScroller->getWidth())
					{
						(*it).mOnscrren = false;
					}
					else
					{
						(*it).mOnscrren = true;
					}

				}
			}
			checkBounds();
		}

		void infinityList::tweenItemPos(const float delta)
		{
			if (mOnScreenItemList.empty() || mIsOnTweenAnimation)
				return;

			mIsOnTweenAnimation = true;

			for (auto it = mOnScreenItemList.begin(); it < mOnScreenItemList.end(); it++)
			{
				auto targetSprite = (*it).mAssociatedSprite;
				auto currentPos = targetSprite->getPosition();
				if (mVertical)
				{
					currentPos.y += delta;
					targetSprite->tweenPosition(currentPos, mTweenAnimationDuration, mTweenAnimationDelay, mTweenAnimationEaseFn, [this, it, targetSprite]()
					{
						if (targetSprite->getPosition().y <= -mIncrementAmount + mStartPositionY || targetSprite->getPosition().y >= mScroller->getHeight())
						{
							(*it).mOnscrren = false;
						}
						else
						{
							(*it).mOnscrren = true;
						}
					});
				}
				else
				{
					currentPos.x += delta;
					targetSprite->tweenPosition(currentPos, mTweenAnimationDuration, mTweenAnimationDelay, mTweenAnimationEaseFn, [this, it, targetSprite]()
					{
						if (targetSprite->getPosition().x <= -mIncrementAmount + mStartPositionX || targetSprite->getPosition().x >= mScroller->getWidth())
						{
							(*it).mOnscrren = false;
						}
						else
						{
							(*it).mOnscrren = true;
						}
					});
				}
				callAfterDelay([this](){checkBounds(); }, mTweenAnimationDelay + mTweenAnimationDuration + 0.1f);

			}

		}

		void infinityList::initFillScreen()
		{
			if (mVertical)
			{
				auto totalHeight = (float)mOnScreenItemSize* mIncrementAmount + mStartPositionY * 2.0f;
				while (totalHeight < mScroller->getHeight())
				{
					mOnScreenItemSize++;
					totalHeight = (float)mOnScreenItemSize* mIncrementAmount + mStartPositionY * 2.0f;
				}
			}
			else{
				auto totalWidth = (float)mOnScreenItemSize* mIncrementAmount + mStartPositionX * 2.0f;
				while (totalWidth < mScroller->getWidth())
				{
					mOnScreenItemSize++;
					totalWidth = (float)mOnScreenItemSize* mIncrementAmount + mStartPositionX * 2.0f;
				}
			}
			mOnScreenItemSize += 2;
		}

		void infinityList::layout()
		{
			assignItems();
		}

		void infinityList::checkBounds()
		{
			if (mOnScreenItemList.empty() || mOnScreenItemList.size() < 1)
				return;

			auto size = mOnScreenItemList.size();

			if (mOnScreenItemList[size - 1].mOnscrren)
			{
				mOnScreenItemList[0].mAssociatedSprite->release();
				mOnScreenItemList.erase(mOnScreenItemList.begin());
				addSpriteToEnd();
			}
			if (mOnScreenItemList[0].mOnscrren)
			{
				mOnScreenItemList[size - 1].mAssociatedSprite->release();
				mOnScreenItemList.pop_back();
				addSpriteToTop();
			}
			mIsOnTweenAnimation = false;
		}

		void infinityList::addSpriteToEnd()
		{
			if (mOnScreenItemList.empty() && mOnScreenItemSize > 2)
				return;

			mTopIndex++;
			mBottomIndex++;
			if (mTopIndex == mItemPlaceHolders.size())
				mTopIndex = 0;
			if (mBottomIndex == mItemPlaceHolders.size())
				mBottomIndex = 0;

			auto &placeHolder = mItemPlaceHolders[mBottomIndex];
			createSprite(placeHolder);
			if (mVertical)
				placeHolder.mAssociatedSprite->setPosition(mStartPositionX, mOnScreenItemList[mOnScreenItemSize - 2].mAssociatedSprite->getPosition().y + mIncrementAmount);
			else
				placeHolder.mAssociatedSprite->setPosition(mOnScreenItemList[mOnScreenItemSize - 2].mAssociatedSprite->getPosition().x + mIncrementAmount, mStartPositionY);
			mOnScreenItemList.push_back(placeHolder);
		}

		void infinityList::addSpriteToTop()
		{
			if (mOnScreenItemList.empty())
				return;

			mTopIndex--;
			mBottomIndex--;
			if (mTopIndex == -1)
				mTopIndex = mItemPlaceHolders.size() - 1;
			if (mBottomIndex == -1)
				mBottomIndex = mItemPlaceHolders.size() - 1;

			auto &placeHolder = mItemPlaceHolders[mTopIndex];
			createSprite(placeHolder);
			if (mVertical)
				placeHolder.mAssociatedSprite->setPosition(mStartPositionX, mOnScreenItemList[0].mAssociatedSprite->getPosition().y - mIncrementAmount);
			else
				placeHolder.mAssociatedSprite->setPosition(mOnScreenItemList[0].mAssociatedSprite->getPosition().x - mIncrementAmount, mStartPositionY);
			mOnScreenItemList.insert(mOnScreenItemList.begin(), placeHolder);
		}

		void infinityList::createSprite(ItemPlaceHolder& placeHolder)
		{
			Sprite* sprite = nullptr;

			if (mCreateItemCallback) sprite = mCreateItemCallback();
			if (sprite){
				sprite->enable(true);
				sprite->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
				sprite->setProcessTouchCallback([this](Sprite* sp, const TouchInfo& ti){
					handleItemTouchInfo(sp, ti);
				});
				sprite->setTapCallback([this, sprite](Sprite* bs, const ci::Vec3f cent){
					Poco::Timestamp::TimeVal nowwwy = Poco::Timestamp().epochMicroseconds();
					float timeDif = (float)(nowwwy - mLastUpdateTime) / 1000000.0f;
					if (timeDif < 0.2f){
						//DS_LOG_INFO("Too soon since the last touch to tap this list!");
						return;
					}
					if (mItemTappedCallback) mItemTappedCallback(sprite, cent);
				});
				mScroller->addChildPtr(sprite);
			}
			else {
				DS_LOG_WARNING("Didn't create a sprite for scroll list! Use the callback and make sprites when we need them!!");
			}

			if (sprite){
				if (mSetDataCallback)
					mSetDataCallback(sprite, placeHolder.mDbId);
				placeHolder.mAssociatedSprite = sprite;
				//placeHolder.mOnscrren = false;
			}
		}

		void infinityList::handleItemTouchInfo(ds::ui::Sprite* bs, const TouchInfo& ti)
		{
			if (bs){
				if (mStateChangeCallback) mStateChangeCallback(bs, ti.mNumberFingers > 0);

				if (mScroller  && ti.mPhase == ds::ui::TouchInfo::Moved && ti.mCurrentGlobalPoint.distance(ti.mStartPoint) > mEngine.getMinTapDistance()){
					if (mStateChangeCallback) mStateChangeCallback(bs, false);
					bs->passTouchToSprite(mScroller, ti);
					return;
				}
			}
		}

	}// namespace ds
}// namespace ui