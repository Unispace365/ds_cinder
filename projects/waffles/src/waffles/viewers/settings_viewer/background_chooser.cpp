#include "stdafx.h"

#include "background_chooser.h"

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/text.h>

#include <ds/ui/button/layout_button.h>

#include "app/waffles_app_defs.h"
#include "waffles/waffles_events.h"

namespace waffles {
BackgroundChooser::BackgroundChooser(ds::ui::SpriteEngine& g)
	: ds::ui::SmartLayout(g, "waffles/settings/background_chooser.xml") {

	setSpriteClickFn("none.the_button", [this] {
		mEngine.getNotifier().notify(RequestBackgroundChange(BACKGROUND_TYPE_NONE, ds::model::ContentModelRef()));
	});

	setSpriteClickFn("particles.the_button", [this] {
		mEngine.getNotifier().notify(RequestBackgroundChange(BACKGROUND_TYPE_PARTICLES, ds::model::ContentModelRef()));
	});
	setSpriteClickFn("media.the_button", [this] {
		auto theBackground = mEngine.mContent.getChildByName("background.user");
		mEngine.getNotifier().notify(RequestBackgroundChange(BACKGROUND_TYPE_USER_MEDIA, theBackground.getChild(0),
															 theBackground.getPropertyInt("pdf_page")));
	});

	setSpriteClickFn("default.the_button", [this] {
		auto theBackground = mEngine.mContent.getChildByName("background");
		mEngine.getNotifier().notify(
			RequestBackgroundChange(BACKGROUND_TYPE_DEFAULT, theBackground.getChildByName("default")));
	});

	setSpriteClickFn("select_media.the_button", [this] {
		auto selectMediaButton = getSprite<ds::ui::LayoutButton>("select_media.the_button");
		if (selectMediaButton) {
			auto possy = selectMediaButton->getPosition();
			if (selectMediaButton->getParent()) {
				possy = selectMediaButton->getParent()->localToGlobal(getPosition());
			}
			ViewerCreationArgs vca =
				ViewerCreationArgs(ds::model::ContentModelRef(), VIEW_TYPE_SELECT_MEDIA_BACKGROUND, possy);
			mEngine.getNotifier().notify(RequestViewerLaunchEvent(vca));
		}
	});

	setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	mLayoutUserType = kFlexSize;
	setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);

	updateButtons();

	mEventClient.listenToEvents<BackgroundChangeComplete>([this](auto& e) { updateButtons(); });
}


void BackgroundChooser::updateButtons() {
	auto noneButton = getSprite<ds::ui::LayoutButton>("none.the_button");
	if (noneButton) noneButton->showUp();

	auto particles = getSprite<ds::ui::LayoutButton>("particles.the_button");
	if (particles) particles->showUp();

	auto media = getSprite<ds::ui::LayoutButton>("media.the_button");
	if (media) media->showUp();

	auto defaultBtn = getSprite<ds::ui::LayoutButton>("default.the_button");
	if (defaultBtn) defaultBtn->showUp();


	auto		theBackground = mEngine.mContent.getChildByName("background");
	auto		theCurrent	  = theBackground.getChildByName("current");
	auto		theUser		  = theBackground.getChildByName("user");
	std::string theTitle;
	if (theUser.getChildren().empty()) {
		theTitle = "Custom Media - None Selected";
	} else {
		theTitle = theUser.getChild(0).getPropertyString("name");
	}

	setSpriteText("media.label_high", theTitle);
	setSpriteText("media.label_normal", theTitle);

	auto theType = theCurrent.getPropertyInt("type");
	switch (theType) {
	case BACKGROUND_TYPE_USER_MEDIA:
		if (media) media->showDown();
		break;

	case BACKGROUND_TYPE_DEFAULT:
		if (defaultBtn) defaultBtn->showDown();
		break;

	case BACKGROUND_TYPE_NONE:
		if (noneButton) noneButton->showDown();
		break;

	case BACKGROUND_TYPE_PARTICLES:
		if (particles) particles->showDown();
		break;
	}
}
} // namespace waffles
