#include "stdafx.h"

#include "scroll_bar.h"

#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_list.h>

#include <algorithm>

namespace ds::ui {

ScrollBar::ScrollBar(ds::ui::SpriteEngine& engine, bool vertical, float uiWidth, float touchPadding, bool autoHide)
  : Sprite(engine)
  , mVertical(vertical)
  , mBackground(nullptr)
  , mNub(nullptr)
  , mMinNubSize(uiWidth)
  , mBarSize(0.0f)
  , mTouchPadding(touchPadding)
  , mScrollPercent(0.0f)
  , mPercentVisible(1.0f)
  , mTouchOffset(0.0f)
  , mAutoHide(autoHide)
  , mAutoHidden(false) {

	// Set some defaults
	// You can change these by getting the nub and background sprites and changing them
	mBackground					= new ds::ui::Sprite(mEngine);
	mBackground->mExportWithXml = false;
	mBackground->setTransparent(false);
	mBackground->setColor(ci::Color(0.1f, 0.1f, 0.1f));
	mBackground->setCornerRadius(uiWidth / 2.0f);
	mBackground->setOpacity(0.7f);
	addChildPtr(mBackground);

	mNub				 = new ds::ui::Sprite(mEngine);
	mNub->mExportWithXml = false;
	mNub->setTransparent(false);
	mNub->setColor(ci::Color(0.9f, 0.9f, 0.9f));
	mNub->setOpacity(0.3f);
	mNub->setSize(uiWidth, uiWidth);
	mNub->setCornerRadius(uiWidth / 2.0f);
	addChildPtr(mNub);

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
	setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) { handleScrollTouch(bs, ti); });

	setSize(mTouchPadding * 2.0f + uiWidth, mTouchPadding * 2.0f + uiWidth);

	if (mAutoHide) {
		doAutoHide(true);
		hide(); // immediately hide, but also call the function to track the state correctly
		setOpacity(0.0f);
		animOpacityStop();
	}
}


void ScrollBar::handleScrollTouch(ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti) {
	if (ti.mFingerIndex == 0) {
		ci::vec3 localPos = globalToLocal(ti.mCurrentGlobalPoint);


		bool  isPerspective = getPerspective();
		float heighty		= getHeight();
		if (!mVertical) {
			heighty = getWidth();
		}

		float barHeight	  = heighty * mPercentVisible;
		float totalHeight = heighty - barHeight;

		float destPercent = 0.0f;

		if (ti.mPhase == ds::ui::TouchInfo::Added) {
			mTouchOffset = barHeight / 2.0f;
			if (mVertical) {
				if (localPos.y > mScrollPercent * totalHeight &&
					localPos.y < mScrollPercent * totalHeight + barHeight) {
					mTouchOffset = localPos.y - mScrollPercent * totalHeight;
				}
			} else {
				if (localPos.x > mScrollPercent * totalHeight &&
					localPos.x < mScrollPercent * totalHeight + barHeight) {
					mTouchOffset = localPos.x - mScrollPercent * totalHeight;
				}
			}
		}

		localPos.y -= mTouchOffset;
		localPos.x -= mTouchOffset;

		if (mVertical) {
			// This may not be right. Feel free to fix, but be sure you get it right and check multiple instances
			if (isPerspective) {
				localPos.y -= getHeight() / 2.0f;
			}

			destPercent = localPos.y / totalHeight;

			if (isPerspective) {
				destPercent = 1.0f - destPercent;
			}

		} else {
			totalHeight = getWidth() - getWidth() * mPercentVisible;
			destPercent = localPos.x / totalHeight;
		}

		destPercent = std::max(destPercent, 0.0f);
		destPercent = std::min(destPercent, 1.0f);

		if (mScrollMoveCallback) {
			mScrollMoveCallback(destPercent);
		}
	}
}


void ScrollBar::setScrollMoveCallback(const std::function<void(float scrollPercent)>& func) {
	mScrollMoveCallback = func;
}

void ScrollBar::scrollUpdated(float percentScrolled, float percentVisible) {
	mScrollPercent = percentScrolled;
	mScrollPercent = std::max(mScrollPercent, 0.0f);
	mScrollPercent = std::min(mScrollPercent, 1.0f);

	mPercentVisible = percentVisible;
	mPercentVisible = std::max(mPercentVisible, 0.0f);
	mPercentVisible = std::min(mPercentVisible, 1.0f);

	updateNubPosition();
}

void ScrollBar::layout() {
	if (mBackground) {
		if (mVertical) {
			mBackground->setSize(std::max(mBarSize, mMinNubSize), getHeight());
			mBackground->setPosition(0.5f * (getWidth() - mBackground->getWidth()), 0.0f);
		} else {
			mBackground->setSize(getWidth(), std::max(mBarSize, mMinNubSize));
			mBackground->setPosition(0.0f, 0.5f * (getHeight() - mBackground->getHeight()));
		}
	}

	updateNubPosition();
}

void ScrollBar::onSizeChanged() {
	layout();
}

void ScrollBar::updateNubPosition() {
	if (mNub && mBackground) {
		if (mVertical) {
			float nubSize = std::max(getHeight() * mPercentVisible, getWidth() - 2 * mTouchPadding);
			mNub->setSize(getWidth() - 2 * mTouchPadding, nubSize);

			if (getPerspective()) {
				mNub->setPosition(mTouchPadding, (1.0f - mScrollPercent) * (getHeight() - nubSize));
			} else {
				mNub->setPosition(mTouchPadding, mScrollPercent * getHeight() - mScrollPercent * mNub->getHeight());
			}
		} else {
			float nubSize = std::max(getWidth() * mPercentVisible, getHeight() - 2 * mTouchPadding);
			mNub->setSize(nubSize, getHeight() - 2 * mTouchPadding);
			mNub->setPosition(getWidth() * mScrollPercent - mScrollPercent * mNub->getWidth(), mTouchPadding);
		}
	}

	if (mAutoHide) {
		if (mPercentVisible >= 0.9999f) {
			doAutoHide(true);
		} else if (mPercentVisible > 0.000000000000f) {
			doAutoHide(false);
		}
	}

	if (mVisualUpdateCallback) {
		mVisualUpdateCallback();
	}
}

void ScrollBar::setVisualUpdateCallback(const std::function<void()>& func) {
	mVisualUpdateCallback = func;
}

ds::ui::Sprite* ScrollBar::getBackgroundSprite() const {
	return mBackground;
}

ds::ui::Sprite* ScrollBar::getNubSprite() const {
	return mNub;
}

void ScrollBar::setMinNubSize(float minNub) {
	mMinNubSize = minNub;
}

void ScrollBar::setBarSize(float barSize) {
	mBarSize = barSize;
	layout();
}

void ScrollBar::setTouchPadding(float touchPadding) {
	mTouchPadding = touchPadding;
	layout();
}

void ScrollBar::linkScrollArea(ds::ui::ScrollArea* area) {
	if (!area) return;

	area->setScrollUpdatedCallback([this](const ds::ui::ScrollArea* scrollArea) {
		scrollUpdated(scrollArea->getScrollPercent(), scrollArea->getVisiblePercent());
	});
	setScrollMoveCallback([area](float scrollPercent) { area->setScrollPercent(scrollPercent); });
}

void ScrollBar::linkScrollList(ds::ui::ScrollList* list) {
	if (!list) return;

	list->setScrollUpdatedCallback([this, list] {
		scrollUpdated(list->getScrollArea()->getScrollPercent(), list->getScrollArea()->getVisiblePercent());
	});

	setScrollMoveCallback([list](float scrollPercent) {
		if (list->getScrollArea()) {
			list->getScrollArea()->setScrollPercent(scrollPercent);
		}
	});
}

void ScrollBar::enableAutoHiding(bool autoHide) {
	mAutoHide = autoHide;
	if (!mAutoHide) {
		doAutoHide(false);
	}
}

void ScrollBar::doAutoHide(bool shouldBeHidden) {
	if (shouldBeHidden) {
		if (!mAutoHidden) {
			mAutoHidden = true;
			tweenOpacity(0.0f, 0.35f, 0.0f, ci::easeNone, [this] { hide(); });
		}
	} else {
		if (mAutoHidden) {
			mAutoHidden = false;
			show();
			tweenOpacity(1.0f, 0.35f);
		}
	}
}

} // namespace ds::ui
