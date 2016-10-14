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

namespace pango {

StoryView::StoryView(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mMessage(nullptr)
	, mPrimaryLayout(nullptr)
	, mImage(nullptr)
	, mPango(nullptr)
{
	//hide();
	//setOpacity(0.0f);
	/*

	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/story_view.xml"), spriteMap);
	mPrimaryLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["root_layout"]);
	mMessage = dynamic_cast<ds::ui::Text*>(spriteMap["message"]);
	mImage = dynamic_cast<ds::ui::Image*>(spriteMap["primary_image"]);

	// calls layout
	setData();
	*/
	animateOn();

	setPosition(100.0f, 100.0f);

	setTransparent(false);
	//setColor(ci::Color(0.5f, 0.5f, 0.5f));
	//setOpacity(0.5f);

//	setScale(0.5f, 0.5f);
	setUseShaderTexture(true);

	setBlendMode(ds::ui::NORMAL);

	kp::pango::CinderPango::setTextRenderer(kp::pango::TextRenderer::FREETYPE);

	//kp::pango::CinderPango::loadFont(ds::Environment::expand("%APP%/data/fonts/CharterITCPro-Black.otf"));
	mPango = kp::pango::CinderPango::create();
	mPango->setDefaultTextSize(48.0f);
	mPango->setMinSize(0, 0);
	mPango->setMaxSize((int)mEngine.getWorldWidth() - 200, (int)mEngine.getWorldHeight());

//	kp::pango::CinderPango::loadFont(ds::Environment::expand("%APP%/data/fonts/CharterITCPro-Regular.otf"));
//	kp::pango::CinderPango::loadFont(ds::Environment::expand("%APP%/data/fonts/NotoSans-Bold.ttf"));

	ds::ui::Text* texty = new ds::ui::Text(mEngine);
	//addChildPtr(texty);
	texty->setFont("noto-regular", 24.0f);
	texty->setText(L"Hello World!");
	texty->setColor(ci::Color::black());
	texty->setPosition(-10.0f, 320.0f);
	//texty->setScale(2.0f);
	//kp::pango::CinderPango::logFontList(true);
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
	if(mPrimaryLayout){
		mPrimaryLayout->runLayout();
	}
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

void StoryView::updateServer(const ds::UpdateParams& p){
	ds::ui::Sprite::updateServer(p);

	// any changes for this frame happen here

	if(mPango){
	//	mPango->setTextAlignment(kp::pango::TextAlignment::JUSTIFY);
		mPango->setText(
			"<markup>"
			"Hello, world "

			"<b>Bold Text</b> "

			" <span foreground='blue'>blue text </span> "
			//"<black>Bold Text</black> "
			//	"<span foreground=\"green\" font=\"24.0\">Green téxt</span> "
			//	"<span foreground=\"red\" font=\"Times 48.0\">Red text</span> "
			//	"<span foreground=\"blue\" font=\"Gravur Condensed Pro 72.0\">AVAVAVA Blue text</span> "
			"<i>Italic Text </i> "
			"<b><i>bold and italic</i></b>"
			"hovedgruppen fra <i>forskjellige</i> <span font_family='Noto Sans'>destinasjoner. Tilknytningsbillett er gyldig inntil 24 timer fr avreise hovedgruppe. </span> \n\nUnicef said 3m"
			" people had been affected and more than <i>1,400</i> had been killed. <b>The government</b> said some 27,000 people remained "
			"trapped "
			"and awaiting <s>help.</s> <sub>Here's some very small text.</sub>"
			" This is great<sup>2</sup>"
			" <span size='xx-large'>HOORAY</span>"
			" The <span font_family='Charter ITC Pro' weight='400' foreground='#3f2f10'>Presidential Election</span> is a <span weight='bold'>Farce</span>"
			"</markup>"
			+ std::to_string(ci::Rand::randFloat())
			);
			/*	*/
			
		// Only renders if it needs to
		mPango->render();
	}
}

void StoryView::drawLocalClient(){
	if(mPango != nullptr) {
		float aaAmount = 0.5f;
		auto tex = mPango->getTexture();
		ci::gl::draw(tex);
	}
}


} // namespace pango
