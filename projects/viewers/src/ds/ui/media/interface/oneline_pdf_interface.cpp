#include "stdafx.h"

#include "oneline_pdf_interface.h"

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/media/interface/thumbnail_bar.h>
#include <ds/ui/media/interface/video_scrub_bar.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/pdf.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>
#include <ds/util/string_util.h>

namespace ds::ui {

OnelinePDFInterface::OnelinePDFInterface(ds::ui::SpriteEngine& eng, const ci::vec2& sizey, const float buttonHeight,
						   const ci::Color buttonColor, const ci::Color backgroundColor)
  : PDFInterface(eng, sizey, buttonHeight, buttonColor, backgroundColor)
  {

	const float padding = sizey.y / 4.0f;
	const float componentsWidth = (mUpButton->getScaleWidth() + padding + mPageCounter->getScaleWidth() + padding +
							   mDownButton->getScaleWidth() + padding + mTouchToggle->getScaleWidth());
	mMinWidth = (componentsWidth * 1.5 + padding +
				 padding * 16.0f // lots of outside padding to account for the page text
	);

	mMaxWidth = mMinWidth;
	
}

void OnelinePDFInterface::updateWidgets() {
	if (mPageCounter) {
		std::wstringstream wss;
		if (mLinkedPDF) {
			wss << mLinkedPDF->getPageNum() << " / " << mLinkedPDF->getPageCount();
		}
		mPageCounter->setText(wss.str());
	}

	if (mThumbsButton && mScrubBar) {
		if (mSourceResource.getChildrenResources().size() < 2) {
			mThumbsButton->hide();
			if (mLinkedPDF && mLinkedPDF->getPageCount() > 1) {
				mScrubBar->show();
				if (getHeight() < mInitialHeight) {
					setSize(getWidth(), mInitialHeight);
				}
			}
		} else {
			mThumbsButton->show();
			mScrubBar->hide();
			if (getHeight() > mInitialHeight) {
				setSize(getWidth(), mInitialHeight);
			}
		}
	}

	if (mLinkedPDF) {
		auto enabled = mLinkedPDF->isEnabled();
		
		if (mLinkedPDF->isEnabled()) {
			mLinkedEnabled = true;
			mTouchToggle->getHighImage().setImageFile(mToggleLockedImage, ds::ui::Image::IMG_CACHE_F);
			mTouchToggle->getNormalImage().setImageFile(mToggleLockedImage, ds::ui::Image::IMG_CACHE_F);
			mTouchToggle->setNormalImageColor(mToggleLockedColor);
			mTouchToggle->setHighImageColor(mToggleUnlockedColor);
		} else if (!mLinkedPDF->isEnabled()) {
			mLinkedEnabled = false;
			mTouchToggle->getHighImage().setImageFile(mToggleUnlockedImage, ds::ui::Image::IMG_CACHE_F);
			mTouchToggle->getNormalImage().setImageFile(mToggleUnlockedImage, ds::ui::Image::IMG_CACHE_F);
			mTouchToggle->setNormalImageColor(mToggleUnlockedColor);
			mTouchToggle->setHighImageColor(mToggleLockedColor);
		}
		
		mTouchToggle->layout();
		mTouchToggle->setScale(mInitialHeight / mTouchToggle->getHeight());
		if (mThumbnailBar) {
			int pageNum = mLinkedPDF->getPageNum() - 1;
			mThumbnailBar->setHighlightedItem(pageNum);
		}
	}
	
	layout();
}

// Layout is called when the size is changed, so don't change the size in the layout
void OnelinePDFInterface::onLayout() {
	const float w		= getWidth();
	const float h		= mInitialHeight;
	const float padding = h / 4.0f;
	if (mUpButton && mDownButton && mPageCounter && mThumbsButton) {

		float componentsWidth = (mUpButton->getScaleWidth() + padding + mPageCounter->getScaleWidth() + padding +
								 mDownButton->getScaleWidth() + padding + mTouchToggle->getScaleWidth());

		if (mThumbsButton->visible()) {
			componentsWidth += padding + mThumbsButton->getScaleWidth();
		}
		

		float margin = ((w - (componentsWidth*1.5+padding)) * 0.5f);
		auto  parent = getParent();

		float xp	 = margin;
		
		float yFudge = h * 0.0f;

		if (mThumbsButton->visible()) {
			mThumbsButton->setPosition(xp, (h * 0.5f) - (mThumbsButton->getScaleHeight() * 0.5f) + yFudge);
			xp += mThumbsButton->getScaleWidth() + padding;
		}

		mPageCounter->setPosition(xp, (h * 0.5f) - (mPageCounter->getHeight() * 0.5f) + yFudge);
		xp += mPageCounter->getScaleWidth() + padding;

		mUpButton->setPosition(xp, (h * 0.5f) - (mUpButton->getScaleHeight() * 0.5f) + yFudge);
		xp += mUpButton->getScaleWidth() + padding;

		
		if (mScrubBar && mScrubBar->visible()) {
			//yFudge = padding / 2.0f;
			mScrubBar->setSize(componentsWidth * 0.25, mScrubBar->getHeight());
			mScrubBar->setPosition(xp, (h * 0.5f) - mScrubBar->getHeight() * 0.5f + yFudge);
			xp += mScrubBar->getWidth() + padding;
		}

		mDownButton->setPosition(xp, (h * 0.5f) - (mDownButton->getScaleHeight() * 0.5f) + yFudge);
		xp += mDownButton->getScaleWidth() + padding;

		mTouchToggle->setPosition(xp, (h * 0.5f) - (mTouchToggle->getScaleHeight() * 0.5f) + yFudge);
		xp += mTouchToggle->getScaleWidth() + padding;
	}


	if (mThumbnailBar) {
		mThumbnailBar->setSize(w, h * 2.0f);
		mThumbnailBar->setPosition(0.0f, -mThumbnailBar->getHeight());
	}
}



} // namespace ds::ui
