#include "stdafx.h"

#include "app/waffles_app_defs.h"
#include "base_element.h"
#include "viewer_controller.h"
#include "ds/util/float_util.h"
#include "waffles/common/ui_utils.h"

namespace waffles {

BaseElement::BaseElement(ds::ui::SpriteEngine& g, std::string eventChannel)
  : BasePanel(g)
  , mCanArrange(true)
  , mCanResize(true)
  , mCanFullscreen(true)
  , mIsFullscreen(false)
  , mCanDetach(false) /* Only enable for elements that are part of a layout */
  , mIsDetached(true) /* Due to touch events being enabled by default */
  , mViewerType(VIEW_TYPE_BASE)
  , mMaxViewersOfThisType(512)
  , mUnfullscreenRect(0.0f, 0.0f, g.getWorldWidth(), g.getWorldHeight())
  , mEventClient(g)
  , mFatalError(false) {

	setChannelName(eventChannel);
	if (eventChannel.empty()) {
		mEventClient.setNotifier(g.getNotifier());
	} else {
		mEventClient.setNotifier(g.getChannel(eventChannel));
	}
	mEventClient.start();

	mAnimDuration = mEngine.getAnimDur();
}

void BaseElement::setMedia(const ds::model::ContentModelRef& newMedia) {
	mMediaRef = newMedia;
	onMediaSet();
}

ci::vec2 BaseElement::getMediaSize() const {
	if (mMediaRef) {
		const auto mediaPropertyKey = ContentUtils::getDefault(mEngine)->getMediaPropertyKey(mMediaRef);
		const auto resource			= mMediaRef.getPropertyResource(mediaPropertyKey);

		if (!resource.empty()) return {resource.getWidth(), resource.getHeight()};
	}
	return {getWidth(), getHeight()};
}

bool BaseElement::canArrange() const {
	return mCanArrange;
}

bool BaseElement::canResize() const {
	return mCanResize;
}

bool BaseElement::canFullScreen() const {
	return mCanFullscreen;
}

void BaseElement::allowFullscreen(bool allow) {
	if (mCanFullscreen == allow) return;
	mCanFullscreen = allow;
	onFullscreenSet();
}

void BaseElement::setIsFullscreen(const bool isFullscreen) {
	if (mIsFullscreen == isFullscreen) return;
	mIsFullscreen = isFullscreen;
	onFullscreenSet();
}

bool BaseElement::getIsFullscreen() const {
	return mIsFullscreen;
}

bool BaseElement::canDetach() const {
	return mCanDetach;
}

void BaseElement::allowDetach(bool allow) {
	if (mCanDetach == allow) return;
	mCanDetach = allow;
	onDetachedSet();
}


bool BaseElement::canAttach() const {
	return mCanAttach;
}

void BaseElement::allowAttach(bool allow) {
	if (mCanAttach == allow) return;
	mCanAttach = allow;
	onDetachedSet();
}

void BaseElement::setIsDetached(const bool isDetached) {
	// Enable/disable touch events but keep constraints.
	if (!mCanDetach || isDetached) {
		enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_SCALE);
	} else {
		disableMultiTouch();
	}

	if (mIsDetached == isDetached) return;
	mIsDetached = isDetached;
	onDetachedSet();
}

bool BaseElement::getIsDetached() const {
	return mIsDetached;
}

int BaseElement::getMaxNumberOfThisType() const {
	return mMaxViewersOfThisType;
}

const std::string& BaseElement::getViewerType() const {
	return mViewerType;
}

bool BaseElement::getIsFatalErrorred() const {
	return mFatalError;
}

void BaseElement::setCloseRequestCallback(std::function<void(void)> func) {
	mCloseRequestCallback = func;
}

void BaseElement::close() {
	if (mCloseRequestCallback) {
		mCloseRequestCallback();
	}
}

void BaseElement::setActivatedCallback(std::function<void(void)> func) {
	mActivatedCallback = func;
}

void BaseElement::animateOn() {
	animateOn(0.0f);
}

void BaseElement::animateOn(const float delay) {
	tweenAnimateOn(true, delay, 0.025f);
}

void BaseElement::setViewerLayer(const int viewerLayer) {
	mViewerLayer = viewerLayer;
	onViewerLayerSet();
}

int BaseElement::getViewerLayer() const {
	return mViewerLayer;
}

void BaseElement::setUnfullscreenRect(ci::Rectf recty) {
	mUnfullscreenRect = recty;
}

ci::Rectf BaseElement::getUnfullscreenRect() const {
	return mUnfullscreenRect;
}

void BaseElement::setToFullscreen(const bool immediate,const bool showController) {
	auto normalLayer = ViewerControllerFactory::getInstanceOf(ci::vec2(), getChannelName())->getNormalLayer();
	const float screenWidth	 = normalLayer->getWidth();  // mDisplaySize.x;
	const float screenHeight = normalLayer->getHeight(); // mDisplaySize.y;
	const float screenAsp	 = screenWidth / screenHeight;

	float viewerAsp	  = getWidth() / getHeight();
	float viewerScale = getScale().x;

	if (viewerScale == 0.0f) viewerScale = 0.001f;
	if (viewerAsp > screenAsp) {
		auto width = screenWidth / viewerScale;
		auto height = screenWidth / viewerAsp;
		auto x		= 0;
		auto y		= screenHeight * 0.5 - height * 0.5;
		if (immediate) {
			setViewerWidth(width);
			setPosition(x, y);
		} else {
			animateWidthTo(width);
			tweenPosition(ci::vec3(x, y, 0.0f),getAnimateDuration(), 0.0f, ci::easeInOutQuad);
		}
	} else {
		auto height	= screenHeight / viewerScale;
		auto width = screenHeight * viewerAsp;
		auto y		= 0;
		auto x		= screenWidth * 0.5 - width * 0.5;
		if (immediate) {
			setViewerHeight(width);
			setPosition(x, y);
		} else {
			animateHeightTo(height);
			tweenPosition(ci::vec3(x, y, 0.0f), getAnimateDuration(), 0.0f, ci::easeInOutQuad);
		}
	}
	setIsFullscreen(true);

}

void BaseElement::setCreationArgs(ViewerCreationArgs args) {
	mCreationArgs = args;

	onCreationArgsSet();
}

void BaseElement::onPanelActivated() {
	if (mActivatedCallback) {
		mActivatedCallback();
	}
}

void BaseElement::onParentSet() {
	/* TODO: this seems fair, but running into crashes, so best I can find to do is remove */
	if (mParent) {
		auto channel = getChannelName();
		if (!channel.empty()) {
			mEngine.timedCallback([this, channel]() { mEventClient.setNotifier(mEngine.getChannel(channel)); }, 0.001);
		}
	}
	
}

} // namespace waffles
