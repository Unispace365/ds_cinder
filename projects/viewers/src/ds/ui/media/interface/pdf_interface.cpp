#include "stdafx.h"

#include "pdf_interface.h"

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/button/toggle_container.h>
#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/media/interface/thumbnail_bar.h>
#include <ds/ui/media/interface/video_scrub_bar.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/pdf.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>
#include <ds/util/string_util.h>

namespace ds::ui {

PDFInterface::PDFInterface(ds::ui::SpriteEngine& eng, const ci::vec2&          sizey, const float buttonHeight,
						   const ci::Color&      buttonColor, const ci::Color& backgroundColor)
  : MediaInterface(eng, ds::Resource::PDF_TYPE, sizey, backgroundColor)
  , mLinkedPDF(nullptr)
  , mLinkedEnabled(false)
  , mUpButton(nullptr)
  , mDownButton(nullptr)
  , mPageCounter(nullptr)
  , mTouchToggle(nullptr)
  , mThumbsButton(nullptr)
  , mScrubBar(nullptr)
  , mInitialHeight(sizey.y)
  , mThumbnailBar(nullptr)
  , mShowingThumbs(false) {

	mCanLock = true;

	//mUpButton = new ds::ui::LayoutButton(mEngine, composeIconPath("ui:media_button:back:normal:file"),
	//							composeIconPath("ui:media_button:back:pressed:file"), 0);
	mUpButton				  = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:back:normal:file",
											 "ui:media_button:back:pressed:file");
	addChildPtr(mUpButton);
	mUpButton->setClickFn([this]() {
		if (mLinkedPDF) {
			if (mLinkedPDF->getPageCount() > 1) {
				mUpButton->enable(false);
				mLinkedPDF->goToPreviousPage();
				updateWidgets();
				mUpButton->callAfterDelay([this] { mUpButton->enable(true); }, 0.15f);
			}
		}
	});

	setButtonColor(mUpButton, buttonColor, buttonColor / 2.0f);


	mDownButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:forward:normal:file",
							   "ui:media_button:forward:pressed:file");
	
	addChildPtr(mDownButton);
	mDownButton->setClickFn([this]() {
		if (mLinkedPDF) {
			if (mLinkedPDF->getPageCount() > 1) {
				mDownButton->enable(false);
				mLinkedPDF->goToNextPage();
				updateWidgets();
				mDownButton->callAfterDelay([this] { mDownButton->enable(true); }, 0.15f);
			}
		}
	});

	setButtonColor(mDownButton, buttonColor, buttonColor / 2.0f);


	mPageCounter				= new ds::ui::Text(mEngine);

	mPageCounter->mLayoutVAlign = ds::ui::LayoutSprite::kMiddle;
	

	if (mPageCounter) {
		addChildPtr(mPageCounter);
		mPageCounter->setTextStyle("viewer:widget");
		auto ts		= mPageCounter->getTextStyle();
		ts.mLeading = 0;
		// mPageCounter->setResizeToText(true);
		mPageCounter->enable(false);
	}
	mToggleLockedImage	 = composeIconPath("ui:media_button:lock:normal:file");
	mToggleUnlockedImage = composeIconPath("ui:media_button:unlock:normal:file");

	mTouchToggle = new ToggleContainer(mEngine,buttonHeight, buttonHeight);
	auto checked = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:lock:normal:file",
								"ui:media_button:lock:pressed:file");
	auto unchecked = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:unlock:normal:file",
								  "ui:media_button:unlock:pressed:file");
	mTouchToggle->setCheckedButton(checked);
	mTouchToggle->setUncheckedButton(unchecked);

	addChildPtr(mTouchToggle);
	checked->setClickFn([this]() { toggleTouch(); });
	unchecked->setClickFn([this]() { toggleTouch(); });
	setButtonColor(checked, buttonColor, buttonColor / 2.0f);
	setButtonColor(unchecked, buttonColor/2.0f, buttonColor);
	

	

	mThumbsButton = createButton(ci::vec2(buttonHeight, buttonHeight), "ui:media_button:thumbnail:normal:file",
								 "ui:media_button:thumbnail:pressed:file");
		
	addChildPtr(mThumbsButton);
	mThumbsButton->setClickFn([this]() {
		mShowingThumbs = !mShowingThumbs;
		if (mShowingThumbs) {
			if (!mThumbnailBar) {
				mThumbnailBar = new ThumbnailBar(mEngine);
				addChildPtr(mThumbnailBar);
				mThumbnailBar->setHighlightColor(ci::Color(0.5f, 0.5f, 0.5f));
				mThumbnailBar->setOpacity(0.0f);
				mThumbnailBar->setData(mSourceResource);
				mThumbnailBar->setThumbnailClickedCallback([this](ds::Resource& reccy) {
					if (mLinkedPDF) {
						mLinkedPDF->setPageNum(reccy.getParentIndex());
					}
				});
			}

			if (mThumbnailBar) {
				mThumbnailBar->show();
				mThumbnailBar->tweenOpacity(1.0f, 0.35f);
			}
		} else {
			if (mThumbnailBar) {
				mThumbnailBar->tweenOpacity(0.0f, 0.35f, 0.0f, ci::easeNone, [this] { mThumbnailBar->hide(); });
			}
		}

		updateWidgets();
	});

	setButtonColor(mThumbsButton, buttonColor, buttonColor / 2.0f);
	
	mScrubBar = new ds::ui::VideoScrubBar(mEngine, sizey.y, mScrubBarHeight, buttonColor);
	addChildPtr(mScrubBar);
	mScrubBar->hide();

	updateWidgets();
	const float padding = sizey.y / 4.0f;

	mMinWidth = (mUpButton->getScaleWidth() + padding + mPageCounter->getScaleWidth() + padding +
				 mDownButton->getScaleWidth() + padding + mTouchToggle->getScaleWidth() +
				 padding * 16.0f // lots of outside padding to account for the page text
	);

	mMaxWidth = mMinWidth;
}

void PDFInterface::linkPDF(ds::ui::IPdf* linkedPDF, const ds::Resource& sourceResource) {
	mLinkedPDF		= linkedPDF;
	mSourceResource = sourceResource;
	if (mLinkedPDF) {
		/*
		mLinkedPDF->setPageChangeCallback([this]{
			updateWidgets();
		});
		*/
	}

	if (mScrubBar) {
		mScrubBar->linkPdf(linkedPDF);
	}

	if (mThumbnailBar) {
		mThumbnailBar->setData(mSourceResource);
	}

	updateWidgets();
}

void PDFInterface::onUpdateServer(const ds::UpdateParams& updateParams) {
	MediaInterface::onUpdateServer(updateParams);
	if (mLinkedPDF) {
		updateWidgets();
	}
}

// Layout is called when the size is changed, so don't change the size in the layout
void PDFInterface::onLayout() {
	const float w		= getWidth();
	const float h		= mInitialHeight;
	const float padding = h / 4.0f;
	if (mUpButton && mDownButton && mPageCounter && mThumbsButton) {

		float componentsWidth = (mUpButton->getScaleWidth() + padding + mPageCounter->getScaleWidth() + padding +
								 mDownButton->getScaleWidth() + padding + mTouchToggle->getScaleWidth());

		if (mThumbsButton->visible()) {
			componentsWidth += padding + mThumbsButton->getScaleWidth();
		}
		float yFudge = 0.0f;
		if (mScrubBar && mScrubBar->visible()) {
			yFudge = padding / 2.0f;
			mScrubBar->setSize(componentsWidth - padding * 2.0f, mScrubBar->getHeight());
			mScrubBar->setPosition(w / 2.0f - mScrubBar->getWidth() / 2.0f, h * 1.5f - mScrubBar->getHeight() / 2.0f);
		}

		float margin = ((w - componentsWidth) * 0.5f);
		float xp	 = margin;

		if (mThumbsButton->visible()) {
			mThumbsButton->setPosition(xp, (h * 0.5f) - (mThumbsButton->getScaleHeight() * 0.5f) + yFudge);
			xp += mThumbsButton->getScaleWidth() + padding;
		}

		mUpButton->setPosition(xp, (h * 0.5f) - (mUpButton->getScaleHeight() * 0.5f) + yFudge);
		xp += mUpButton->getScaleWidth() + padding;

		mPageCounter->setPosition(xp, (h * 0.5f) - (mPageCounter->getHeight() * 0.5f) + yFudge);
		xp += mPageCounter->getScaleWidth() + padding;

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

void PDFInterface::updateWidgets() {
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
				if (getHeight() < mInitialHeight * 2.0f) {
					setSize(getWidth(), mInitialHeight * 2.0f);
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
		if (mLinkedPDF->isEnabled()) {
			mLinkedEnabled = true;
			mTouchToggle->setChecked(true);
			setButtonColor(mTouchToggle->getCheckedButton(), mToggleLockedColor, mToggleUnlockedColor);
		} else if (!mLinkedPDF->isEnabled()) {
			mLinkedEnabled = false;
			mTouchToggle->setChecked(false);
			setButtonColor(mTouchToggle->getUncheckedButton(), mToggleUnlockedColor, mToggleLockedColor);
		}
		//mTouchToggle->layout();
		//mTouchToggle->setScale(mInitialHeight / mTouchToggle->getHeight());
		if (mThumbnailBar) {
			int pageNum = mLinkedPDF->getPageNum() - 1;
			mThumbnailBar->setHighlightedItem(pageNum);
		}
	}

	layout();
}

void PDFInterface::setPageFont(const std::string &fontName, double fontSize) {
	if (mPageCounter) {
		mPageCounter->setFont(fontName);
		mPageCounter->setFontSize(fontSize);
	}
}

void PDFInterface::setAllowTouchToggle(const bool allowTouchToggling) {
	if (allowTouchToggling) {
		DS_LOG_WARNING("PDFInterface: touch toggling is currently not allowed");
	}
}

ds::ui::Sprite* PDFInterface::getScrubBarBackground() {
	if (!mScrubBar) return nullptr;
	return mScrubBar->getBacker();
}

void PDFInterface::setToggleLockedImage(const std::string& imgPath) {
	mToggleLockedImage = imgPath;
	updateWidgets();
	mTouchToggle->setScale(mInitialHeight / mTouchToggle->getHeight());
	
}

void PDFInterface::setToggleUnlockedImage(const std::string& imgPath) {
	mToggleUnlockedImage = imgPath;
	updateWidgets();
	mTouchToggle->setScale(mInitialHeight / mTouchToggle->getHeight());
	
}

void PDFInterface::setToggleLockedColor(const ci::ColorAf& color) {
	mToggleLockedColor = color;
	updateWidgets();
}

void PDFInterface::setToggleUnlockedColor(const ci::ColorAf& color) {
	mToggleUnlockedColor = color;
	updateWidgets();
}

void PDFInterface::toggleTouch() {
	if (mLinkedPDF) {
		if (mLinkedPDF->isEnabled()) {
			stopTouch();
			setLocked(false);
		} else {
			startTouch();
			setLocked(true);
		}
	}
}

void PDFInterface::startTouch() {
	if (mLinkedPDF) {
		mLinkedPDF->enable(true);
		mLinkedPDF->showLinks();
	}
	updateWidgets();
}


void PDFInterface::stopTouch() {
	if (mLinkedPDF) {
		mLinkedPDF->enable(false);
		mLinkedPDF->hideLinks();
	}
	updateWidgets();
}

ds::ui::Sprite* PDFInterface::getScrubBarProgress() {
	if (!mScrubBar) return nullptr;
	return mScrubBar->getProgress();
}

void PDFInterface::addNubToScrubBar(ds::ui::Sprite* newNub) {
	if (!mScrubBar) return;
	mScrubBar->addNub(newNub);
}

} // namespace ds::ui
