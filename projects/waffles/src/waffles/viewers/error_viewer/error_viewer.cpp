#include "stdafx.h"

#include "error_viewer.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/util/string_util.h>

#include "app/waffles_app_defs.h"



namespace waffles {

ErrorViewer::ErrorViewer(ds::ui::SpriteEngine& g)
	: BaseElement(g)
	, mPrimaryLayout(nullptr) {

	mViewerType			  = VIEW_TYPE_ERROR;
	mMaxViewersOfThisType = 10;
	mCanResize			  = false;
	mCanArrange			  = false;
	mCanFullscreen		  = false;

	mPrimaryLayout = new ds::ui::SmartLayout(mEngine, "waffles/error/error_viewer.xml");
	addChildPtr(mPrimaryLayout);


	mPrimaryLayout->setSpriteClickFn("close_button.the_button", [this] {
		if (mCloseRequestCallback) mCloseRequestCallback();
	});
	mPrimaryLayout->runLayout();

	const float startWidth	= mPrimaryLayout->getWidth();
	const float startHeight = mPrimaryLayout->getHeight();
	mContentAspectRatio		= startWidth / startHeight;

	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth, startHeight);

	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);

	setAnimateOnScript(mEngine.getWafflesSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));
}

void ErrorViewer::onMediaSet() {
	if (mPrimaryLayout) {
		mPrimaryLayout->setContentModel(mMediaRef);

		const float startWidth	= mPrimaryLayout->getWidth();
		const float startHeight = mPrimaryLayout->getHeight();
		mContentAspectRatio		= startWidth / startHeight;

		setSize(startWidth, startHeight);
		setSizeLimits();
		setViewerSize(startWidth, startHeight);
	}
}

void ErrorViewer::onLayout() {
	if (mPrimaryLayout) {
		mPrimaryLayout->runLayout();
	}
}


} // namespace waffles
