#include "keyboardexample_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>

#include <ds/ui/media/media_viewer.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include "ui/story/story_view.h"
#include "ui/text_test/text_test_view.h"

#include <ds/ui/soft_keyboard/entry_field.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>
#include <ds/ui/soft_keyboard/soft_keyboard_defs.h>
#include <ds/ui/soft_keyboard/soft_keyboard_button.h>
#include <ds/ui/soft_keyboard/soft_keyboard_builder.h>

namespace example {

KeyboardExample::KeyboardExample()
	: inherited(ds::RootList()

	// Note: this is where you'll customize the root list
								.ortho() 
								.pickColor()

								.persp() 
								.perspFov(60.0f)
								.perspPosition(ci::vec3(0.0, 0.0f, 10.0f))
								.perspTarget(ci::vec3(0.0f, 0.0f, 0.0f))
								.perspNear(0.0002f)
								.perspFar(20.0f)

								.ortho() ) 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mTouchDebug(mEngine)
{


	/*fonts in use */
	mEngine.editFonts().install("Noto Sans Bold", "noto-bold");
	mEngine.editFonts().install("Noto Sans", "noto-thin");

	enableCommonKeystrokes(true);
}

void KeyboardExample::setupServer(){


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
			p.mTarget = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f);
			p.mFarPlane = clippFar;
			p.mFov = fov;
			p.mPosition = ci::vec3(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, mEngine.getWorldWidth() / 2.0f);
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
	rootSprite.addChildPtr(new StoryView(mGlobals));

	ds::ui::EntryField* ef = new ds::ui::EntryField(mEngine, ds::ui::EntryFieldSettings());
	rootSprite.addChildPtr(ef);
	ef->focus();
	ef->setPosition(100.0f, 100.0f);

	ds::ui::SoftKeyboardSettings sks;
	sks.mKeyTextOffset = mGlobals.getSettingsLayout().getSize("keyboard:text_offset", 0, ci::vec2());
	ds::ui::SoftKeyboard* sk = ds::ui::SoftKeyboardBuilder::buildStandardKeyboard(mEngine, sks);
	rootSprite.addChildPtr(sk);
	sk->setPosition(mEngine.getWorldWidth() / 2.0f - sk->getWidth()/2.0f, mEngine.getWorldHeight() / 2.0f - sk->getHeight()/2.0f);
	sk->setKeyPressFunction([this, sk, ef](const std::wstring& character, ds::ui::SoftKeyboardDefs::KeyType keyType){
		if(keyType == ds::ui::SoftKeyboardDefs::kEnter){
			sk->resetCurrentText();
		}
		mEngine.getNotifier().notify(KeyPressedEvent(sk->getCurrentText()));
		//ef->setCurrentText(sk->getCurrentText());
		ef->keyPressed(character, keyType);
	});

	rootSprite.addChildPtr(new TextTest(mGlobals));
}

void KeyboardExample::update() {
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

void KeyboardExample::keyDown(ci::app::KeyEvent event){
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

void KeyboardExample::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void KeyboardExample::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void KeyboardExample::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void KeyboardExample::fileDrop(ci::app::FileDropEvent event){
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
CINDER_APP(example::KeyboardExample, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))