#include "stdafx.h"

#include "base_element.h"
#include "app/waffles_app_defs.h"

namespace waffles {

BaseElement::BaseElement(ds::ui::SpriteEngine& g)
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

void BaseElement::setCreationArgs(ViewerCreationArgs args) {
	mCreationArgs = args;

	onCreationArgsSet();
}

void BaseElement::onPanelActivated() {
	if (mActivatedCallback) {
		mActivatedCallback();
	}
}

void BaseElement::onParentSet()
{
	auto channel = getChannelName();
	if (!channel.empty()) {
		mEngine.timedCallback([this, channel]() {
			mEventClient.setNotifier(mEngine.getChannel(channel));
			}, 0.001);
	}
}

} // namespace waffles
