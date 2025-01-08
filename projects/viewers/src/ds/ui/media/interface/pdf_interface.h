#pragma once
#ifndef DS_UI_MEDIA_VIEWER_PDF_INTERFACE
#define DS_UI_MEDIA_VIEWER_PDF_INTERFACE

#include <ds/data/resource.h>

#include "ds/ui/media/media_interface.h"

namespace ds::ui {

class ImageButton;
class Text;
class IPdf;
class ThumbnailBar;
class VideoScrubBar;

/**
 * \class PDFInterface
 *			Implements page up/down, page count
 *			Note: for PDF thumbnail viewer to show up, the PDF needs to be loaded via a Resource
 *					that has a children vector of resources of the thumbnails set, and the children need to have the
 *correct parentIndex (i.e. page number) set.
 */
class PDFInterface : public MediaInterface {
  public:
	PDFInterface(ds::ui::SpriteEngine& eng, const ci::vec2& interfaceSize, const float buttonHeight,
				 const ci::Color buttonColor, const ci::Color backgroundColor);

	virtual void linkPDF(ds::ui::IPdf* linkedPDF, const ds::Resource& sourceResource);
	virtual void updateWidgets();
	virtual void setPageFont(std::string fontName, float fontSize);

	/// For visual customization - don't release these sprites from here
	virtual void				   addNubToScrubBar(ds::ui::Sprite* newNub);
	virtual ds::ui::ImageButton* getUpButton() { return mUpButton; }
	virtual ds::ui::ImageButton*	   getDownButton() { return mDownButton; }
	virtual ds::ui::ImageButton*	   getTouchToggle() { return mTouchToggle; }
	virtual ds::ui::ImageButton*		   getThumbsButton() { return mThumbsButton; }
	virtual ds::ui::VideoScrubBar*	   getScrubBar() { return mScrubBar; }
	virtual ds::ui::Sprite*				   getScrubBarProgress();
	virtual ds::ui::Text*				   getPageCounter() { return mPageCounter; }
	virtual ds::ui::Sprite*				   getScrubBarBackground();

	virtual void toggleTouch(); // what the "touch lock" does
	virtual void	 startTouch();	// pdf is tappable to go forwards/back
	virtual void	 stopTouch();	// pdf is not tappable

  protected:
	virtual void onUpdateServer(const ds::UpdateParams& updateParams) override;
	virtual void onLayout() override;

	ds::ui::IPdf* mLinkedPDF;
	ds::Resource  mSourceResource;
	bool		  mLinkedEnabled;

	ds::ui::ImageButton*   mUpButton;
	ds::ui::ImageButton*   mDownButton;
	ds::ui::Text*		   mPageCounter;
	ds::ui::ImageButton*   mTouchToggle;
	ds::ui::ImageButton*   mThumbsButton;
	ds::ui::VideoScrubBar* mScrubBar;
	float				   mInitialHeight;

	ds::ui::ThumbnailBar* mThumbnailBar;
	bool				  mShowingThumbs;
};

} // namespace ds::ui

#endif
