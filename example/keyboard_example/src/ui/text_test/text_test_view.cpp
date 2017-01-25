#include "text_test_view.h"

#include <poco/Timestamp.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

namespace example {

TextTest::TextTest(Globals& g)
	: inherited(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mMessage(nullptr)
{

	mMessage = mGlobals.getText("sample:config").createMultiline(mEngine, this);
	if(mMessage){
		std::wstring theText = ds::wstr_from_utf8(mGlobals.getSettingsLayout().getText("text:test:text", 0, ""));
		mMessage->setText(theText);

		Poco::Timestamp::TimeVal before = Poco::Timestamp().epochMicroseconds();

		ci::vec2 resizeSize = mGlobals.getSettingsLayout().getSize("text:test:resize", 0, ci::vec2(0.0f, 0.0f));
		mMessage->setResizeLimit(resizeSize.x, resizeSize.y);

		std::cout << mMessage->getWidth() << " " << mMessage->getPositionForCharacterIndex(0) << std::endl;

		Poco::Timestamp::TimeVal after = Poco::Timestamp().epochMicroseconds();

		float delty = (float)(after - before) / 1000000.0f;
		std::cout << "Layout time: " << delty << std::endl;

		std::vector<ci::Rectf> characterPositions;
		for(int i = 0; i < theText.size() + 1; i++){
			ci::Rectf possy = mMessage->getRectForCharacterIndex(i);
			characterPositions.push_back(possy);
		}

		for(int i = 0; i < theText.size(); i++){
			ci::Rectf possy = characterPositions[i];
			//ci::vec2 possyTwo = ci::vec2(0.0f, 0.0f);
			//if(i + 1 < theText.size()){
			//	possyTwo = characterPositions[i + 1];
			//}
			ds::ui::Sprite* overlay = new ds::ui::Sprite(mEngine);
			addChildPtr(overlay);
			overlay->setTransparent(false);
			overlay->setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
			overlay->setPosition(possy.x1, possy.y1);
			overlay->setSize(possy.getWidth(), possy.getHeight());
			overlay->setOpacity(0.5f);
			mCharacterOverlays.push_back(overlay);
		}
	}

	layout();

	setPosition(mGlobals.getSettingsLayout().getSize("text:test:offset", 0, ci::vec2()));

}

void TextTest::onAppEvent(const ds::Event& in_e){
// 	if(in_e.mWhat == IdleEndedEvent::WHAT()){
// 		const IdleEndedEvent& e((const IdleEndedEvent&)in_e);
// 		animateOn();
// 	} else if(in_e.mWhat == IdleStartedEvent::WHAT()){
// 		animateOff();
// 	} else if(in_e.mWhat == KeyPressedEvent::WHAT()){
// 		if(mMessage){
// 			const KeyPressedEvent& e((const KeyPressedEvent&)in_e);
// 			mMessage->setText(e.mFullString);
// 		}
// 	}

	// If you have an event that is dispatched when new content is queryied, you could map that here.
	//if(in_e.mWhat == StoryContentUpdated::WHAT()){
	//	setData();
	//}
}

void TextTest::onSizeChanged(){

}

void TextTest::layout(){
// 	if(mMessage){
// 		mMessage->setPosition(100.0f, 200.0f);
// 	}
}

} // namespace example