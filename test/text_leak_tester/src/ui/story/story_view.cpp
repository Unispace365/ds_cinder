#include "stdafx.h"

#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ds/ui/interface_xml/interface_xml_importer.h"

namespace downstream {

StoryView::StoryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mTheText(nullptr)
	, mASprite(nullptr)
{
//	callAfterDelay([this] { restartSprite(); }, 2.0f);
	callAfterDelay([this] { restartText(); }, 2.0f);
}

static const char alphanum[] =
"0123456789"
"!@#$%^&*"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

int stringLength = sizeof(alphanum) - 1;

char genRandom(){
	return alphanum[rand() % stringLength];
}

void StoryView::restartText() {
	/*
	if(mTheText) {
		auto tt = mTheText;
		mTheText->callAfterDelay([this, tt] { tt->release(); }, 0.01f);
		mTheText = nullptr;
	}
	*/

	if(!mTheText) {
		mTheText = new ds::ui::Text(mEngine);
		mTheText->setFont("Arial Bold");
		addChildPtr(mTheText);
		mTheText->setResizeLimit(mEngine.getWorldWidth(), mEngine.getWorldHeight());
	}

	int numOfChars = rand() % 10000;
	std::string theText = "";
	for(int i = 0; i < numOfChars; i++) {
		theText += genRandom();
	}

	mTheText->setText(theText);
	mTheText->setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
	mTheText->setFontSize(ci::randFloat(4.0f, 40.0f));

	mTheText->getWidth();
//	callAfterDelay([this] {
//		mEngine.getNotifier().notify(RequestAppExitEvent());
//	}, 1.0);
	callAfterDelay([this] { restartText(); }, 0.001f);

}


void StoryView::restartSprite() {
	if(mASprite) {
		auto tt = mASprite;
		mASprite->callAfterDelay([this, tt] { tt->release(); }, 0.1f);
	}

	mASprite = nullptr;

	mASprite = new ds::ui::Sprite(mEngine);
	addChildPtr(mASprite);
	mASprite->setTransparent(false);
	mASprite->setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
	mASprite->setSize(ci::randFloat(0.1f, mEngine.getWorldWidth()), ci::randFloat(0.1f, mEngine.getWorldHeight()));

	callAfterDelay([this] { restartSprite(); }, 0.0001f);

}
} // namespace downstream

