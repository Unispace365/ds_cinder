#include "story_view.h"

#include <Poco/LocalDateTime.h>

#include <Cinder/Rand.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ds/ui/interface_xml/interface_xml_importer.h"
#include "ds/ui/layout/layout_sprite.h"

namespace pango {

StoryView::StoryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mMessage(nullptr)
	, mImage(nullptr)
	, mPangoText(nullptr)
	, mFakeCursor(nullptr)
{


	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/story_view.xml"), spriteMap);
	auto mPrimaryLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["layout"]);

	mFakeCursor = new ds::ui::Sprite(mEngine);
	mFakeCursor->setSize(4.0f, 100.0f);
	mFakeCursor->setTransparent(false);
	mFakeCursor->setColor(ci::Color::white());
	mFakeCursor->setOpacity(0.5f);
	addChildPtr(mFakeCursor);

	mPangoText = dynamic_cast<ds::ui::Text*>(spriteMap["message_b"]);
	if(mPangoText){
		mFullText = mPangoText->getText();
		mPangoText->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
			auto touchPos = bs->globalToLocal(ti.mCurrentGlobalPoint);
			
			if(mPangoText){
				int indexy = mPangoText->getCharacterIndexForPosition(ci::vec2(touchPos));
				auto recty = mPangoText->getRectForCharacterIndex(indexy);
				auto globby = mPangoText->localToGlobal(ci::vec3(recty.x1, recty.y1, 0.0f));
				auto loccy = globalToLocal(globby);
				if(mFakeCursor){
					mFakeCursor->setPosition(loccy);
					mFakeCursor->setSize(recty.getWidth(), recty.getHeight());
				}
			}
		});

		randomizeText();

	}

	if(mPrimaryLayout){
		mPrimaryLayout->runLayout();
	}

	animateOn();

}

void StoryView::randomizeText(){
	mFullText += L" " + std::to_wstring(ci::randFloat());
	static bool switcher = false;
	switcher = !switcher;
	if(switcher){
		mPangoText->setText(mFullText);
	} else {
		mPangoText->setText(L"Hello there!");
	}
	mPangoText->callAfterDelay([this]{
		randomizeText();
	}, 5.0f);
}

void StoryView::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == IdleEndedEvent::WHAT()){
		const IdleEndedEvent& e((const IdleEndedEvent&)in_e);
		//animateOn();
	} else if(in_e.mWhat == IdleStartedEvent::WHAT()){
		//animateOff();
	}

	// If you have an event that is dispatched when new content is queryied, you could map that here.
	if(in_e.mWhat == StoryDataUpdatedEvent::WHAT()){
		setData();
	}
}

void StoryView::setData() {
	// update view to match new content
	// See story_query from where this content is sourced from
	// In a real case, you'd likely have a single story ref for this instance and use that data
	if(!mGlobals.mAllData.mStories.empty()){

		auto storyRef = mGlobals.mAllData.mStories.front();

		if(mMessage){
			// Map the content from the app to the view sprites
			mMessage->setText(storyRef.getTitle());
		}

		if(mImage && storyRef.getPrimaryResource().getType() == ds::Resource::IMAGE_TYPE){
			mImage->setImageResource(storyRef.getPrimaryResource());
		}
	}

	layout();
}

void StoryView::layout(){
}

void StoryView::animateOn(){
	show();
	tweenOpacity(1.0f, mGlobals.getAnimDur());

	// Recursively animate on any children, including the primary layout
	tweenAnimateOn(true, 0.0f, 0.05f);
}

void StoryView::animateOff(){
	tweenOpacity(0.0f, mGlobals.getAnimDur(), 0.0f, ci::EaseNone(), [this]{hide(); });
}

void StoryView::onUpdateServer(const ds::UpdateParams& p){

	// any changes for this frame happen here

	if(mPangoText && mMessage){
	//	mPango->setTextAlignment(kp::pango::TextAlignment::JUSTIFY);

		std::string theText =
			"<markup>"
			//"<span font='HelveticaNeueLT Std UtlLt Ext'>"
			"Hello, world "
			"Here's a <span font='HelveticaNeueLT Std Cn Bold'>bunch more text</span> to simulate a lot of the more text that the other option had for a <span font='HelveticaNeueLT Std Lt Light'>lot more</span> of the text and stuff and this sentence has a lot of and's and other and's and other things of that nature and such. "
			"<span font='HelveticaNeueLT Std Med Cn Medium'>Bold Text</span> "

			"<span foreground='blue'>blue text</span> "
			//"<black>Bold Text</black> "
			//	"<span foreground=\"green\" font=\"24.0\">Green téxt</span> "
			//	"<span foreground=\"red\" font=\"Times 48.0\">Red text</span> "
			//	"<span foreground=\"blue\" font=\"Gravur Condensed Pro 72.0\">AVAVAVA Blue text</span> "
			//"<i>Italic Text </i> "
			//"<b><i>bold and italic</i></b>"
			//"hovedgruppen fra <i>forskjellige</i> <span font_family='Noto Sans'>destinasjoner. Tilknytningsbillett er gyldig inntil 24 timer fr avreise hovedgruppe. </span> \n\nUnicef said 3m"
			//" people had been affected and more than <i>1,400</i> had been killed. <b>The government</b> said some 27,000 people remained "
			"trapped "
			//"and awaiting <s>help.</s> <sub>Here's some very small text.</sub>"
			//" This is great<sup>2</sup>"
			//" <span size='xx-large'>HOORAY</span>"
			//" The <span font_family='Charter ITC Pro' weight='400' foreground='#3f2f10'>Presidential Election</span> is a <span weight='bold'>Farce</span>"
			+std::to_string(ci::Rand::randFloat())
			//+ "</span>"
			+"</markup>";

		//mPangoText->setText(theText);
			/*	*/
			
		// Only renders if it needs to
		//mPango->render();

	//	mMessage->setText(theText);
	}
}



} // namespace pango


