#include "stdafx.h"

#include "settings_viewer.h"

#include <ds/app/environment.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
#include "waffles/waffles_events.h"
#include "waffles/viewers/settings_viewer/ambient_chooser.h"
#include "waffles/viewers/settings_viewer/background_chooser.h"

namespace waffles {

SettingsViewer::SettingsViewer(ds::ui::SpriteEngine& g)
	: BaseElement(g)
	, mEventClient(g) {

	mMaxViewersOfThisType = 1;
	mViewerType			  = VIEW_TYPE_SETTINGS;
	mCanArrange			  = false;

	mRootLayout = new ds::ui::SmartLayout(mEngine, "waffles/settings/settings_viewer.xml");
	addChildPtr(mRootLayout);

	mRootLayout->setSpriteClickFn("close_button.the_button", [this]() {
		if (mCloseRequestCallback) mCloseRequestCallback();
		ViewerCreationArgs vca =
			ViewerCreationArgs(ds::model::ContentModelRef(), VIEW_TYPE_LAUNCHER, getCenterPosition());
		mEngine.getNotifier().notify(RequestViewerLaunchEvent(vca));
	});

	auto holder = mRootLayout->getSprite("holder");
	if (holder) {
		if (mEngine.getAppSettings().getBool("ambient:enabled", 0, true))
			holder->addChildPtr(new AmbientChooser(mEngine));
		holder->addChildPtr(new BackgroundChooser(mEngine));
	}

	mRootLayout->runLayout();

	const float startWidth	= mRootLayout->getWidth();
	const float startHeight = mRootLayout->getHeight();
	mContentAspectRatio		= startWidth / startHeight;

	BasePanel::setAbsoluteSizeLimits(ci::vec2(startWidth, startHeight),
									 ci::vec2(mEngine.getWorldWidth(), mEngine.getWorldHeight()));

	mEventClient.listenToEvents<PresentationStatusUpdatedEvent>([this](auto& e) { updateUi(); });

	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth, startHeight);

	setAnimateOnScript(mEngine.getAppSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));

	updateUi();
}

void SettingsViewer::onLayout() {
	if (mRootLayout) {
		mRootLayout->setSize(getWidth(), getHeight());
		mRootLayout->runLayout();
	}
}


void SettingsViewer::updateUi() {
	// update?

	layout();
}
} // namespace waffles
