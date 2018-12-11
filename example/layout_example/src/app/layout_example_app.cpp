#include "layout_example_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/text.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>

#include <ds/ui/layout/smart_layout.h>

namespace example {

layout_example::layout_example()
	: inherited() 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
{


	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");
}

void layout_example::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();
	mQueryHandler.runInitialQueries();


	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
	// add sprites

	ds::ui::LayoutSprite* rootLayout = new ds::ui::LayoutSprite(mEngine);
	rootLayout->enable(true);
	rootLayout->enableMultiTouch(ds::ui::MULTITOUCH_CAN_SCALE | ds::ui::MULTITOUCH_CAN_POSITION);
	rootLayout->setTouchScaleMode(true);
	rootLayout->setProcessTouchCallback([this, rootLayout](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		if(rootLayout->getWidth() < 300.0f){
			rootLayout->setSize(300.0f, rootLayout->getHeight());
		}
		rootLayout->runLayout();
	});
	rootLayout->setColor(ci::Color(0.0f, 0.0f, 0.5f));
	rootLayout->setTransparent(false);
	rootLayout->setSize(400.0f, 600.0f);
	rootLayout->setPosition(100.0f, 100.0f);
	rootLayout->setSpacing(10.0f);
	rootLayout->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	rootLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);

	ds::ui::Sprite* topFixed = new ds::ui::Sprite(mEngine, 50.0f, 100.0f);
	topFixed->setColor(ci::Color(0.5f, 0.0f, 0.0f));
	topFixed->setTransparent(false);
	topFixed->mLayoutUserType = ds::ui::LayoutSprite::kFixedSize;
	topFixed->mLayoutHAlign = ds::ui::LayoutSprite::kCenter;
	rootLayout->addChildPtr(topFixed);



	// Nested horizontal layout
	ds::ui::LayoutSprite* medFixed = new ds::ui::LayoutSprite(mEngine);
	medFixed->setSize(50.0f, 100.0f);
	medFixed->setColor(ci::Color(0.0f, 0.5f, 0.0f));
	medFixed->setTransparent(false);
	medFixed->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	medFixed->setLayoutType(ds::ui::LayoutSprite::kLayoutHFlow);
	medFixed->mLayoutLPad = 50.0f;
	rootLayout->addChildPtr(medFixed);

	ds::ui::Text* horizText = mGlobals.getText("sample:config").create(mEngine, medFixed);
	horizText->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	horizText->mLayoutLPad = 10.0f;
	horizText->mLayoutRPad = 10.0f;
	horizText->mLayoutBPad = 10.0f;
	horizText->mLayoutTPad = 10.0f;
	horizText->setText("I'm in the middle of your layout. Hey.");
	horizText->setResizeLimit(200.0f);

	ds::ui::Sprite* spaceFiller = new ds::ui::Sprite(mEngine, 100.0f, medFixed->getHeight());
	spaceFiller->mLayoutUserType = ds::ui::LayoutSprite::kStretchSize;
	spaceFiller->setColor(ci::Color(0.2f, 0.2f, 0.2f));
	spaceFiller->setTransparent(false);
	medFixed->addChildPtr(spaceFiller);


	// bottom part of the vertical layout
	ds::ui::LayoutSprite* botStretch = new ds::ui::LayoutSprite(mEngine);
	botStretch->setColor(ci::Color(0.3f, 0.2f, 0.4f));
	botStretch->setTransparent(false);
	botStretch->setSize(50.0f, 100.0f);
	botStretch->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	botStretch->mLayoutLPad = 10.0f;
	botStretch->mLayoutRPad = 10.0f;
	rootLayout->addChildPtr(botStretch);

	ds::ui::Image* imagey = new ds::ui::Image(mEngine);
	imagey->setImageFile("%APP%/data/images/Colbert.png");
	imagey->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	imagey->mLayoutLPad = 10.0f;
	imagey->mLayoutRPad = 10.0f;
	rootLayout->addChildPtr(imagey);


	ds::ui::Text* mt = mGlobals.getText("sample:config").create(mEngine, rootLayout);
	mt->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	mt->mLayoutLPad = 10.0f;
	mt->mLayoutRPad = 10.0f;
	mt->mLayoutBPad = 10.0f;
	mt->mLayoutTPad = 10.0f;
	mt->setText("Hello, and welcome to the bottom of the layout area. Thank you for staying.");

	rootLayout->runLayout();
	rootSprite.addChildPtr(rootLayout);



	// Separate horizontal layout ----------------------------------------------------
	ds::ui::LayoutSprite* horizontalLayout = new ds::ui::LayoutSprite(mEngine);
	horizontalLayout->enable(true);
	horizontalLayout->enableMultiTouch(ds::ui::MULTITOUCH_CAN_SCALE | ds::ui::MULTITOUCH_CAN_POSITION);
	horizontalLayout->setTouchScaleMode(true);
	horizontalLayout->setProcessTouchCallback([this, horizontalLayout](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		if(horizontalLayout->getHeight() < 300.0f){
			horizontalLayout->setSize(horizontalLayout->getWidth(), 300.0f);
		}
		horizontalLayout->runLayout();
	});
	horizontalLayout->setColor(ci::Color(0.5f, 0.0f, 0.5f));
	horizontalLayout->setTransparent(false);
	horizontalLayout->setSize(400.0f, 600.0f);
	horizontalLayout->setPosition(800.0f, 100.0f);
	horizontalLayout->setSpacing(10.0f);
	horizontalLayout->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	horizontalLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutHFlow);
	horizontalLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);

	ds::ui::Text* sampley = mGlobals.getText("sample:config").create(mEngine, horizontalLayout);
	sampley->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	sampley->mLayoutLPad = 10.0f;
	sampley->mLayoutRPad = 10.0f;
	sampley->mLayoutBPad = 10.0f;
	sampley->mLayoutTPad = 10.0f;
	sampley->setText("Designing success through guessing and checking.");
	sampley->setResizeLimit(200.0f);

	ds::ui::Image* sampleImage = new ds::ui::Image(mEngine);
	sampleImage->setImageFile("%APP%/data/images/Colbert.png");
	sampleImage->mLayoutUserType = ds::ui::LayoutSprite::kFlexSize;
	sampleImage->mLayoutLPad = 10.0f;
	sampleImage->mLayoutRPad = 10.0f;
	sampleImage->mLayoutTPad = 10.0f;
	sampleImage->mLayoutBPad = 10.0f;
	horizontalLayout->addChildPtr(sampleImage);

	horizontalLayout->runLayout();
	horizontalLayout->runAnimationScript("slide:-100.0, 0.0; fade:-1.0; ease:outBack; duration:1.0; delay:2.0");
	rootSprite.addChildPtr(horizontalLayout);


	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(&rootSprite, ds::Environment::expand("%APP%/data/layouts/layout_view.xml"), spriteMap);

	auto generatedLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["root_layout"]);
	if(generatedLayout){
		generatedLayout->runLayout();
		generatedLayout->tweenAnimateOn(true);
		generatedLayout->tweenAnimateOn(true, 0.5f, 0.1f);
	}

	std::map<std::string, ds::ui::Sprite*>	spriteMapTwo;
	ds::ui::XmlImporter::loadXMLto(&rootSprite, ds::Environment::expand("%APP%/data/layouts/layout_alignments.xml"), spriteMapTwo);

	auto generatedLayoutTwo = dynamic_cast<ds::ui::LayoutSprite*>(spriteMapTwo["root_layout"]);
	if(generatedLayoutTwo){
		generatedLayoutTwo->runLayout();
		generatedLayoutTwo->tweenAnimateOn(true, 0.5f, 0.1f);
	}


	ds::ui::SmartLayout* sl = new ds::ui::SmartLayout(mEngine, "smart_layout.xml");
	sl->listenToEvents<IdleStartedEvent>([sl](const IdleStartedEvent& e){ sl->setSpriteText("event_text", "Idling!"); });
	sl->listenToEvents<IdleEndedEvent>([sl](const IdleEndedEvent& e){ sl->setSpriteText("event_text", "Not Idling!"); });
	rootSprite.addChildPtr(sl);
	sl->tweenAnimateOn(true, 0.75f, 0.1f);
}


void layout_example::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;


}


void layout_example::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		//paths.push_back((*it).string());

		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
		mv->initialize();
		mEngine.getRootSprite().addChildPtr(mv);
	}
}

} // namespace example

// This line tells Cinder to actually create the application
CINDER_APP(example::layout_example, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)),
		   [&](ci::app::App::Settings* settings) { settings->setBorderless(true); })

