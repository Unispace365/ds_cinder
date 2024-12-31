#include "stdafx.h"

#include "base_element.h"
#include "viewer_controller.h"
#include "app/waffles_app_defs.h"

namespace waffles {

BaseElement::BaseElement(ds::ui::SpriteEngine& g, std::string eventChannel)
	: BasePanel(g)
	, mCanArrange(true)
	, mCanResize(true)
	, mCanFullscreen(true)
	, mIsFullscreen(false)
	, mViewerType(VIEW_TYPE_BASE)
	, mMaxViewersOfThisType(512)
	, mUnfullscreenRect(0.0f, 0.0f, g.getWorldWidth(), g.getWorldHeight())
	, mFatalError(false)
	, mEventClient(g){

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



bool BaseElement::canArrange() {
	return mCanArrange;
}

bool BaseElement::canResize() {
	return mCanResize;
}

bool BaseElement::canFullScreen() {
	return mCanFullscreen;
}

void BaseElement::setIsFullscreen(const bool isFullscreen) {
	mIsFullscreen = isFullscreen;

	onFullscreenSet();
}

bool BaseElement::getIsFullscreen() {
	return mIsFullscreen;
}

const int BaseElement::getMaxNumberOfThisType() {
	return mMaxViewersOfThisType;
}

const std::string& BaseElement::getViewerType() {
	return mViewerType;
}

bool BaseElement::getIsFatalErrored() {
	return mFatalError;
}

void BaseElement::setCloseRequestCallback(std::function<void(void)> func) {
	mCloseRequestCallback = func;
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

const int BaseElement::getViewerLayer() {
	return mViewerLayer;
}

void BaseElement::setUnfullscreenRect(ci::Rectf recty) {
	mUnfullscreenRect = recty;
}

ci::Rectf BaseElement::getUnfullscreenRect() {
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
