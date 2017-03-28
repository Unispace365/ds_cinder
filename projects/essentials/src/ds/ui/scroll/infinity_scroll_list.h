#pragma once
#ifndef DS_UI_SCROLL_INFINITY_SCROLL_LIST
#define DS_UI_SCROLL_INFINITY_SCROLL_LIST

#include <ds/ui/sprite/sprite.h>
#include <functional>
#include <ds/ui/sprite/gradient_sprite.h>
#include <Poco/Timestamp.h>
#include <ds/ui/touch/momentum.h>

namespace ds{
	namespace ui{

		class infinityList : public ds::ui::Sprite {
		public:

			infinityList(ds::ui::SpriteEngine& engine, const float startWidth, const float startHeight, const bool verticalScrolling = true);

			void								setContent(const std::vector<int>& dbIds);

			// OPTIONAL (but highly recommended): A row has been tapped, set this function to handle it
			void								setItemTappedCallback(const std::function<void(ds::ui::Sprite* bs, const ci::vec3& cent)> &func);

			// REQUIRED: When we need to create a new sprite, respond with a new sprite of your custom type
			void								setCreateItemCallback(const std::function<ds::ui::Sprite*() > &func);

			// REQUIRED: When a sprite needs data assigned (coming onscreen for the first time for example). May need to cast the sprite to your custom type
			void								setDataCallback(const std::function<void(ds::ui::Sprite*, const int dbId) > &func);

			// OPTIONAL: During animate on, you can call custom animation code here with delay (set member properties for delay if desired, defaults=0.0 seconds)
			void								setAnimateOnCallback(const std::function<void(ds::ui::Sprite*, const float delay)>&func);

			// OPTIONAL: If you want to show highlighted states you can react here
			void								setStateChangeCallback(const std::function<void(ds::ui::Sprite*, const bool highlighted)>&func);

			// OPTIONAL: Called whenever the scroll changes position (could be quite a lot). Useful if you want to add scroll bars or update other ui
			void								setScrollUpdatedCallback(const std::function<void(void)> &func);

			// REQUIRED TO LOOK OK: 
			// @param startPositionX Where to start the items horizontally
			// @param startPositionY Where to start the items Vertically
			// @param incremenetAmount How much distance between the start of one item and the start of the next item
			// @param fill_from_top Whether to align to the bottom of the scroll area or the top. For instance, if there's not enough items to fill the whole space, will start filling and align to the bottom if this param is false.
			void								setLayoutParams(const float startPositionX, const float startPositionY, const float incremenetAmount);

			void								setTweenAnimationParams(const float duration, const float delay = 0.0f, const ci::EaseFn fn= ci::EaseNone());
			void								nextItem(const float duration = -1.0f);
			void								previousItem(const float duration = -1.0f);
			void								turnOnStepSwipe();
			
			// Call this to initialize the list at a certain point
			void								initItemStart(int itemNum);
			// Call this to jump to a certain item in the list
			void								jumpItem(int itemNum, const float duration = -1.0f);

			// Returns the positions of items currently on screen
			std::vector<int>					getOnScreenItemsPos();

		protected:
			// A helper so we only have to show the visible results at one time (instead of creating a zillion sprites)
			struct ItemPlaceHolder	{

				ItemPlaceHolder(const int dbId, float x = 0.0f, float y = 0.0f, ds::ui::Sprite *associatedSprite = nullptr, bool onScreen = false)
					: mDbId(dbId)
					, mAssociatedSprite(associatedSprite)
					, mOnscrren(onScreen)
				{
				};

				int							mDbId;
				ds::ui::Sprite*				mAssociatedSprite;
				bool						mOnscrren;
			};

			virtual void						updateServer(const ds::UpdateParams& p);
			virtual void						clearItems();
			virtual void						assignItems();
			void								handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti);
			void								itemPosUpdated(const float delta);
			void								tweenItemPos(const float delta, float duration = -1.0f);
			void								initFillScreen();
			void								layout();
			void								checkBounds();
			void								addSpriteToEnd();
			void								addSpriteToTop();
			void								createSprite(ItemPlaceHolder&);
			void								handleItemTouchInfo(ds::ui::Sprite* bs, const TouchInfo& ti);
			std::vector<ItemPlaceHolder>		mItemPlaceHolders;

			ds::Momentum						mSpriteMomentum;

			ds::ui::Sprite*						mScroller;
			std::vector<ItemPlaceHolder>		mOnScreenItemList;
			float								mPadding;
			bool								mScrollable;

			bool								mVertical;

			float								mStartPositionY;
			float								mStartPositionX;
			float								mIncrementAmount;

			int									mOnScreenItemSize;
			int									mTopIndex;
			int									mBottomIndex;
			bool								mIsOnTweenAnimation;
			bool								mIsTurnOnStepSwipe;

			float								mTweenAnimationDuration;
			float								mTweenAnimationDelay;
			ci::EaseFn							mTweenAnimationEaseFn;

			Poco::Timestamp::TimeVal			mLastUpdateTime;


			std::function<void(ds::ui::Sprite*, const ci::vec3& cent)>	mItemTappedCallback;
			std::function<ds::ui::Sprite* ()>							mCreateItemCallback;
			std::function<void(ds::ui::Sprite*, const int dbId)>		mSetDataCallback;
			std::function<void(ds::ui::Sprite*, const float delay)>		mAnimateOnCallback;
			std::function<void(ds::ui::Sprite*, const bool highli)>		mStateChangeCallback;
			std::function<void()>										mScrollUpdatedCallback;
		};
	} // namespace ds
}

#endif