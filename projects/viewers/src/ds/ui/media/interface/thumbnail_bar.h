#pragma once
#ifndef DS_UI_VIEWERS_INTERFACE_THUMBNAIL_BAR
#define DS_UI_VIEWERS_INTERFACE_THUMBNAIL_BAR

#include <ds/data/resource.h>
#include <ds/ui/scroll/scroll_list.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite.h>

namespace ds { namespace ui {

	/**
	 * \class ThumbnailBar
	 *			Show the thumbnails for a Resource (like a PDF) in a bar and get callbacks for when those thumbnails are
	 *clicked
	 */
	class ThumbnailBar : public ds::ui::Sprite {
	  public:
		ThumbnailBar(ds::ui::SpriteEngine& se);

		/// Get notified when a thumbnail has been clicked
		/// The resource that's returned is of the child element that was clicked on
		void setThumbnailClickedCallback(std::function<void(ds::Resource&)> func) { mClickedCallback = func; }

		/// Set the content. Exclusively uses the children of the Resource
		void setData(ds::Resource& parentResource);

		void layout();

		void setHighlightColor(const ci::Color& highlightColor);
		void setHighlightedItem(const int itemIndex);

		void setFixedWidth(bool isFixedWidth) { mFixedWidth = isFixedWidth; }
		void setFilledBackgrounds(bool isFilledBgs) { mFilledBackgrounds = isFilledBgs; }

		ds::ui::ScrollList* getScrollList() { return mFileList; }

	  protected:
		virtual void onSizeChanged();

		void setImageSize(ds::ui::Sprite* sp);

		void updateHighlight();

		ds::ui::ScrollList*				   mFileList;
		ds::Resource					   mSourceResource;
		std::map<int, ds::Resource>		   mInfoMap;
		std::map<Image*, ds::Resource>	   mImageMap;
		std::function<void(ds::Resource&)> mClickedCallback;
		float							   mPadding;
		float							   mItemSize;
		float							   mSourceAspect;
		bool							   mFixedWidth		  = false;
		bool							   mFilledBackgrounds = false;

		ci::Color mHighlightColor;
		int		  mHighlightItemIndex;
	};

}} // namespace ds::ui

#endif
