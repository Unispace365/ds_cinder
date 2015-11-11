#include "layout_example_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/multiline_text.h>
#include <ds/ui/sprite/text.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/Rand.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include "ui/layout/layout_sprite.h"

namespace example {

layout_example::layout_example()
	: inherited(ds::RootList()

	// Note: this is where you'll customize the root list
								.ortho() 
								.pickColor()

								.persp() 
								.perspFov(60.0f)
								.perspPosition(ci::Vec3f(0.0, 0.0f, 10.0f))
								.perspTarget(ci::Vec3f(0.0f, 0.0f, 0.0f))
								.perspNear(0.0002f)
								.perspFar(20.0f)

								.ortho() ) 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mTouchDebug(mEngine)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "noto-bold");

	enableCommonKeystrokes(true);
}

void layout_example::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	mGlobals.initialize();
	mQueryHandler.runInitialQueries();

	const int numRoots = mEngine.getRootCount();
	int numPlacemats = 0;
	for(int i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;

		ds::ui::Sprite& rooty = mEngine.getRootSprite(i);
		if(rooty.getPerspective()){
			const float clippFar = 10000.0f;
			const float fov = 60.0f;
			ds::PerspCameraParams p = mEngine.getPerspectiveCamera(i);
			p.mTarget = ci::Vec3f(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f);
			p.mFarPlane = clippFar;
			p.mFov = fov;
			p.mPosition = ci::Vec3f(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, mEngine.getWorldWidth() / 2.0f);
			mEngine.setPerspectiveCamera(i, p);
		} else {
			mEngine.setOrthoViewPlanes(i, -10000.0f, 10000.0f);
		}

		rooty.clearChildren();
	}

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
	// add sprites

	LayoutSprite* rootLayout = new LayoutSprite(mEngine);
	rootLayout->enable(true);
	rootLayout->enableMultiTouch(ds::ui::MULTITOUCH_CAN_SCALE | ds::ui::MULTITOUCH_CAN_POSITION);
	rootLayout->setTouchScaleMode(true);
	rootLayout->setProcessTouchCallback([this, rootLayout](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		rootLayout->runLayout();
	});
	rootLayout->setColor(ci::Color(0.0f, 0.0f, 0.5f));
	rootLayout->setTransparent(false);
	rootLayout->setSize(400.0f, 600.0f);
	rootLayout->setPosition(100.0f, 100.0f);
	rootLayout->setSpacing(10.0f);
	rootLayout->mLayoutUserType = LayoutSprite::kFlexSize;

	ds::ui::Sprite* topFixed = new ds::ui::Sprite(mEngine, 50.0f, 100.0f);
	topFixed->setColor(ci::Color(0.5f, 0.0f, 0.0f));
	topFixed->setTransparent(false);
	rootLayout->addChildPtr(topFixed);


	ds::ui::Sprite* medFixed = new ds::ui::Sprite(mEngine, 50.0f, 100.0f);
	medFixed->setColor(ci::Color(0.0f, 0.5f, 0.0f));
	medFixed->setTransparent(false);
	rootLayout->addChildPtr(medFixed);


	LayoutSprite* botStretch = new LayoutSprite(mEngine);
	botStretch->setColor(ci::Color(0.3f, 0.2f, 0.4f));
	botStretch->setTransparent(false);
	botStretch->setSize(50.0f, 100.0f);
	botStretch->mLayoutUserType = LayoutSprite::kFlexSize;
	botStretch->mLayoutLPad = 10.0f;
	botStretch->mLayoutRPad = 10.0f;
	rootLayout->addChildPtr(botStretch);

	ds::ui::Image* imagey = new ds::ui::Image(mEngine);
	imagey->setImageFile("%APP%/data/images/Colbert.png");
	imagey->mLayoutUserType = LayoutSprite::kFlexSize;
	imagey->mLayoutLPad = 10.0f;
	imagey->mLayoutRPad = 10.0f;
	rootLayout->addChildPtr(imagey);


	ds::ui::MultilineText* mt = mGlobals.getText("sample:config").createMultiline(mEngine, rootLayout);
	mt->mLayoutUserType = LayoutSprite::kFlexSize;
	mt->mLayoutLPad = 10.0f;
	mt->mLayoutRPad = 10.0f;
	mt->mLayoutBPad = 10.0f;
	mt->mLayoutTPad = 10.0f;
	mt->setText("Hello, and welcome to the bottom of the layout area. Thank you for staying.");

	rootLayout->runLayout();
	rootSprite.addChildPtr(rootLayout);


}

void layout_example::update() {
	inherited::update();

	if( mEngine.isIdling() && !mIdling ){
		//Start idling
		mIdling = true;
		mEngine.getNotifier().notify( IdleStartedEvent() );
	} else if ( !mEngine.isIdling() && mIdling ){
		//Stop idling
		mIdling = false;
		mEngine.getNotifier().notify( IdleEndedEvent() );
	}

}

void layout_example::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();

	// Shows all enabled sprites with a label for class type
	} else if(event.getCode() == KeyEvent::KEY_f){

		const int numRoots = mEngine.getRootCount();
		int numPlacemats = 0;
		for(int i = 0; i < numRoots - 1; i++){
			mEngine.getRootSprite(i).forEachChild([this](ds::ui::Sprite& sprite){
				if(sprite.isEnabled()){
					sprite.setTransparent(false);
					sprite.setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
					sprite.setOpacity(0.95f);

					ds::ui::Text* labelly = mGlobals.getText("media_viewer:title").create(mEngine, &sprite);
					labelly->setText(typeid(sprite).name());
					labelly->enable(false);
					labelly->setColor(ci::Color::black());
				} else {

					ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(&sprite);
					if(!texty || (texty && texty->getColor() != ci::Color::black())) sprite.setTransparent(true);
				}
			}, true);
		}
	}
}

void layout_example::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void layout_example::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void layout_example::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
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
CINDER_APP_BASIC(example::layout_example, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))