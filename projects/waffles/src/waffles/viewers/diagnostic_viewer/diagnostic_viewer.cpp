#include "stdafx.h"

#include "diagnostic_viewer.h"


#include <ds/app/environment.h>
#include <ds/data/color_list.h>
#include <ds/data/resource.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/image_button.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
#include "diagnostic_test.h"


#include "ds/ui/interface_xml/interface_xml_importer.h"

#include <shellapi.h>

namespace waffles {

DiagnosticViewer::DiagnosticViewer(ds::ui::SpriteEngine& g)
	: BaseElement(g)
	, mPrimaryLayout(nullptr) {

	mViewerType			  = VIEW_TYPE_DIAGNOSTIC;
	mMaxViewersOfThisType = 1;
	mCanResize			  = false;
	mCanArrange			  = false;
	mCanFullscreen		  = false;

	mPrimaryLayout = new ds::ui::SmartLayout(mEngine, "waffles/diagnostics/diagnostic_viewer.xml");
	addChildPtr(mPrimaryLayout);

	mPrimaryLayout->setSpriteClickFn("run_button.the_button", [this] { startDiagnostics(); });
	mPrimaryLayout->setSpriteClickFn("show_logs.the_button", [this] { showLogs(); });
	mPrimaryLayout->setSpriteClickFn("close_button.the_button", [this] {
		if (mCloseRequestCallback) mCloseRequestCallback();
	});


	int numConns = (int)mEngine.getAppSettings().countSetting("diagnostics:connectivity");
	for (int i = 0; i < numConns; i++) {
		std::string url = mEngine.getAppSettings().getString("diagnostics:connectivity", i, "");
		if (!url.empty()) {
			addTest(DiagnosticTest::TEST_TYPE_URL, url);
		}
	}

	addTest(DiagnosticTest::TEST_TYPE_IP);
	addTest(DiagnosticTest::TEST_TYPE_MEMORY_INFO);
	addTest(DiagnosticTest::TEST_TYPE_SCREEN_SETUP);
	addTest(DiagnosticTest::TEST_TYPE_SOUND_CARD_GUIDS);
	addTest(DiagnosticTest::TEST_TYPE_TOUCH);
	addTest(DiagnosticTest::TEST_TYPE_VIDEO_CARD_INFO);

	mPrimaryLayout->runLayout();

	const float startWidth	= mPrimaryLayout->getWidth();
	const float startHeight = mPrimaryLayout->getHeight();
	mContentAspectRatio		= startWidth / startHeight;

	setSize(startWidth, startHeight);
	setSizeLimits();
	setViewerSize(startWidth, startHeight);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);

	setAnimateOnScript(mEngine.getAppSettings().getString("animation:viewer_on", 0, "grow; ease:outQuint"));
}

void DiagnosticViewer::addTest(const int type, const std::string& url /*= ""*/) {
	if (!mPrimaryLayout) return;
	auto holder = mPrimaryLayout->getSprite("holder");
	if (!holder) return;

	DiagnosticTest* dt = new DiagnosticTest(mEngine, holder->getWidth(), type, url);
	holder->addChildPtr(dt);
	mTests.push_back(dt);
	dt->setLayoutCallback([this] {
		if (!mPrimaryLayout) return;

		mPrimaryLayout->runLayout();

		const float startWidth	= mPrimaryLayout->getWidth();
		const float startHeight = mPrimaryLayout->getHeight();
		mContentAspectRatio		= startWidth / startHeight;

		setSize(startWidth, startHeight);
		setSizeLimits();
		setViewerSize(startWidth, startHeight);
	});
	dt->setCompleteCallback([this] {
		mNumCompletedTests++;
		if (mNumCompletedTests == mTests.size()) {
			testsComplete();
		}
	});
}

void DiagnosticViewer::startDiagnostics() {
	mNumCompletedTests = 0;
	for (auto it : mTests) {
		it->runTest();
	}
}

void DiagnosticViewer::showLogs() {
	std::wstring wloc = ds::wstr_from_utf8(ds::Environment::expand("%APP%/logs/"));
	LPCWSTR		 loccy(wloc.c_str());
	ShellExecute(NULL, L"open", loccy, NULL, NULL, SW_SHOWDEFAULT);
}

void DiagnosticViewer::testsComplete() {
	int numSuccess = 0;
	for (auto it : mTests) {
		if (it->passed()) numSuccess++;
	}
	std::stringstream ss;
	ss << "Tests complete! " << numSuccess << " of " << mTests.size() << " were successful";
	DS_LOG_INFO(ss.str());

	if (mPrimaryLayout) {
		mPrimaryLayout->setSpriteText("status_text", ss.str());
		layout();
	}
}

void DiagnosticViewer::onLayout() {
	if (mPrimaryLayout) {
		mPrimaryLayout->runLayout();
	}
}


} // namespace waffles
