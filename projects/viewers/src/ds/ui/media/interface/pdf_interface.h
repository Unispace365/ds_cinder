#pragma once
#ifndef DS_UI_MEDIA_VIEWER_PDF_INTERFACE
#define DS_UI_MEDIA_VIEWER_PDF_INTERFACE

#include <ds/data/resource.h>

#include "ds/ui/media/media_interface.h"

namespace ds::ui {

class LayoutButton;
class ToggleContainer;
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
	PDFInterface(ds::ui::SpriteEngine& eng, const ci::vec2&          interfaceSize, const float buttonHeight,
				 const ci::Color&      buttonColor, const ci::Color& backgroundColor);

	virtual void linkPDF(ds::ui::IPdf* linkedPDF, const ds::Resource& sourceResource);
	virtual void updateWidgets();
	virtual void setPageFont(const std::string &fontName, double fontSize);

	/// For visual customization - don't release these sprites from here
	virtual void				   addNubToScrubBar(ds::ui::Sprite* newNub);
	virtual ds::ui::LayoutButton* getUpButton() { return mUpButton; }
	virtual ds::ui::LayoutButton*		   getDownButton() { return mDownButton; }
	virtual ds::ui::ToggleContainer*		   getTouchToggle() { return mTouchToggle; }
	virtual ds::ui::LayoutButton*		   getThumbsButton() { return mThumbsButton; }
	virtual ds::ui::VideoScrubBar*	   getScrubBar() { return mScrubBar; }
	virtual ds::ui::Sprite*				   getScrubBarProgress();
	virtual ds::ui::Text*				   getPageCounter() { return mPageCounter; }
	virtual ds::ui::Sprite*				   getScrubBarBackground();
	virtual void						   setToggleLockedImage(const std::string& imgPath);
	virtual void						   setToggleUnlockedImage(const std::string& imgPath);
	virtual void						   setToggleLockedColor(const ci::ColorAf& color);
	virtual void						   setToggleUnlockedColor(const ci::ColorAf& color);	

	void setAllowTouchToggle(const bool allowTouchToggling) override;
	void toggleTouch() override; // what the "touch lock" does
	void startTouch() override;  // pdf is tappable to go forwards/back
	void stopTouch() override;   // pdf is not tappable

  protected:
	virtual void onUpdateServer(const ds::UpdateParams& updateParams) override;
	virtual void onLayout() override;

	ds::ui::IPdf* mLinkedPDF;
	ds::Resource  mSourceResource;
	bool		  mLinkedEnabled;
	bool		  mAbleToTouchToggle;

	ds::ui::LayoutButton*  mUpButton;
	ds::ui::LayoutButton*  mDownButton;
	ds::ui::Text*		   mPageCounter;
	ds::ui::ToggleContainer*  mTouchToggle;
	ds::ui::LayoutButton*  mThumbsButton;
	ds::ui::VideoScrubBar* mScrubBar;
	float				   mInitialHeight;

	std::string mToggleLockedImage	 = "%APP%/data/images/media_interface/touch_locked.png";
	std::string mToggleUnlockedImage = "%APP%/data/images/media_interface/touch_unlocked.png";
	ci::ColorAf mToggleLockedColor	 = ci::ColorAf(0.0f, 0.0f, 0.0f, 0.1f);
	ci::ColorAf mToggleUnlockedColor = ci::ColorAf(1.0f, 1.0f, 1.0f, 0.1f);

	ds::ui::ThumbnailBar* mThumbnailBar;
	bool				  mShowingThumbs;

};

} // namespace ds::ui

#endif
