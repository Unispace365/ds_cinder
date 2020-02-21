#pragma once
#ifndef DS_UI_SCROLL_SCROLL_LIST
#define DS_UI_SCROLL_SCROLL_LIST

#include <ds/ui/sprite/sprite.h>

#include <Poco/Timestamp.h>

namespace ds{
namespace ui{
class ScrollArea;



	/**
	* ScrollList is an advanced Scroll Area that handles an arbitrarily large set of items.
	* ScrollList handles keeping a cache of placeholders and assigning sprites to onscreen items.
	* This assumes that you can refer to your content by integers only, so you may have to keep a map in your super class.
	*
	* NOTE: In perspective sprites, the list will fill up from the bottom. If you need to modify this, leave filling from the bottom the default.
	*/

	class ScrollList : public ds::ui::Sprite {
	public:
		ScrollList(ds::ui::SpriteEngine& engine, const bool verticalScroll = true);

		void						setContent(const std::vector<int>& dbIds);

		/// OPTIONAL (but highly recommended): A row has been tapped, set this function to handle it
		void						setItemTappedCallback(const std::function<void(ds::ui::Sprite* bs, const ci::vec3& cent)> &func);

		/// REQUIRED: When we need to create a new sprite, respond with a new sprite of your custom type
		void						setCreateItemCallback(const std::function<ds::ui::Sprite*() > &func);

		/// REQUIRED: When a sprite needs data assigned (coming onscreen for the first time for example). May need to cast the sprite to your custom type
		void						setDataCallback(const std::function<void(ds::ui::Sprite*, const int dbId) > &func);

		/// OPTIONAL: During animate on, you can call custom animation code here with delay (set member properties for delay if desired, defaults=0.0 seconds)
		void						setAnimateOnCallback(const std::function<void(ds::ui::Sprite*, const float delay)>&func);

		/// OPTIONAL: If you want to show highlighted states you can react here
		void						setStateChangeCallback(const std::function<void(ds::ui::Sprite*, const bool highlighted)>&func);

		/// OPTIONAL: Called whenever the scroll changes position (could be quite a lot). Useful if you want to add scroll bars or update other ui
		void						setScrollUpdatedCallback(const std::function<void(void)> &func);

		/// Animates the current items onscreen only
		void						animateItemsOn();

		/// When using animateItemsOn()
		void						setAnimateOnParams(const float startDelay, const float deltaDelay);

		/// REQUIRED TO LOOK OK: 
		/// \param startPositionX Where to start the items horizontally
		/// \param startPositionY Where to start the items Vertically
		/// \param incremenetAmount How much distance between the start of one item and the start of the next item
		/// \param fill_from_top Whether to align to the bottom of the scroll area or the top. For instance, if there's not enough items to fill the whole space, will start filling and align to the bottom if this param is false.
		void						setLayoutParams(const float startPositionX, const float startPositionY, const float incremenetAmount, const bool fill_from_top = true);


		//When mOriginTop==true, shift items to top of scroll list
		void						pushItemsTop();
		/// Use caution when modifying the scroll area
		/// Recommend only using this to reset the scroll position and change the fade graphics
		/// Setting your own scroll position callback will break the scroll list
		ds::ui::ScrollArea*			getScrollArea(){ return mScrollArea; }

		/// calls a function for each sprite that's currently onscreen and in reserve
		void						forEachLoadedSprite(std::function<void(ds::ui::Sprite*)> function);

		/// Makes a grid that goes left-to-right, then top-to-bottom with a set advance increment between the columns and rows
		void						setGridLayout(const bool doGrid, const ci::vec2& gridIncrement);

		/// Makes a grid targetting a specific number of rows or columns
		/// If using a vertical scroll list, use targetColumn; if using a horizontal scroll list, use targetRow
		/// Padding will be the spacing between the items
		void						setMatrixLayout(const bool doGrid, const int targetRow, const int targetColumn, const float padding);

		/// Run the layout of children, just in case you need to force the layout. Most changes induce this automatically.
		virtual void				layout();

		bool isVerticalScroll() const { return mVerticalScrolling; }

	protected:

		/// We only create enough sprites that are onscreen at one time.
		/// Here's how this crap works (roughly in order):
		//		- Set Data: clear out old data, and build a list of placeholders. These are just the data (product), a position and a pointer to a sprite
		//		- layoutItems: place the placeholders on a virtual grid.
		//		- assignItems: figure out which placeholders should be onscreen and associate sprites with them. New sprites can be created here if needeed.
		//						extra sprites are kept in mReserveI tems.

		/// A helper so we only have to show the visible results at one time (instead of creating a zillion sprites)
		struct ItemPlaceHolder	{

			ItemPlaceHolder(const int dbId, float x = 0.0f, float y = 0.0f, ds::ui::Sprite *associatedSprite = nullptr)
				: mDbId(dbId)
				, mX(x)
				, mY(y)
				, mAssociatedSprite(associatedSprite)
			{
			};

			int							mDbId;
			float						mX;
			float						mY;
			ds::ui::Sprite*				mAssociatedSprite;
		};


		virtual void						onSizeChanged();
		virtual void						layoutItems();
		virtual void						layoutItemsGrid();
		virtual void						layoutItemsMatrix();

		virtual void						clearItems();
		virtual void						assignItems();

		void								handleItemTouchInfo(ds::ui::Sprite* bs, const TouchInfo& ti);

		std::vector<ItemPlaceHolder>		mItemPlaceHolders;
		std::vector<ds::ui::Sprite*>		mReserveItems;

		ds::ui::ScrollArea*					mScrollArea;
		ds::ui::Sprite*						mScrollableHolder;
		const bool							mVerticalScrolling;

		/// for laying out items
		float								mStartPositionY;
		float								mStartPositionX;
		float								mIncrementAmount;
		bool								mFillFromTop;
		bool								mGridLayout;
		bool								mSpecialLayout;
		ci::vec2							mGridIncrement;

		/// for animate on
		float								mAnimateOnDeltaDelay;
		float								mAnimateOnStartDelay;

		std::function<void(ds::ui::Sprite*, const ci::vec3& cent)>	mItemTappedCallback;
		std::function<ds::ui::Sprite* ()>							mCreateItemCallback;
		std::function<void(ds::ui::Sprite*, const int dbId)>		mSetDataCallback;
		std::function<void(ds::ui::Sprite*, const float delay)>		mAnimateOnCallback;
		std::function<void(ds::ui::Sprite*, const bool highli)>		mStateChangeCallback;
		std::function<void()>										mScrollUpdatedCallback;

		/// Track update time so touches can't happen while the list is being dragged cause of lazy fingers
		Poco::Timestamp::TimeVal			mLastUpdateTime;

		int									mTargetRow;
		int									mTargetColumn;
		float								mMatrixPadding;
	};
}
}

#endif
