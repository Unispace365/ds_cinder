#include "keyboardexample_app.h"


#include <cinder/Rand.h> 

#include <ds/app/environment.h>
#include <ds/app/engine/engine.h>

#include <ds/ui/media/media_viewer.h>

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
	: ds::App() 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mTouchDebug(mEngine)
	, mSoftKeyboard(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().registerFont("Noto Sans Bold", "noto-bold");
	mEngine.editFonts().registerFont("Noto Sans", "noto-thin");
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

	ds::ui::EntryFieldSettings efs;
	efs.mCursorOffset.x = 0.0f;
	ds::ui::EntryField* ef = new ds::ui::EntryField(mEngine, efs);
	rootSprite.addChildPtr(ef);
	ef->focus();
	ef->setPosition(100.0f, 100.0f);

	ds::ui::SoftKeyboardSettings sks;
	sks.mGraphicKeys = true;
	sks.mKeyTextOffset = mGlobals.getSettingsLayout().getVec2("keyboard:text_offset", 0, ci::vec2());
	//mSoftKeyboard = ds::ui::SoftKeyboardBuilder::buildExtendedKeyboard(mEngine, sks);
	//mSoftKeyboard = ds::ui::SoftKeyboardBuilder::buildStandardKeyboard(mEngine, sks);
	//mSoftKeyboard = ds::ui::SoftKeyboardBuilder::buildLowercaseKeyboard(mEngine, sks);
	mSoftKeyboard = ds::ui::SoftKeyboardBuilder::buildFullKeyboard(mEngine, sks);
	rootSprite.addChildPtr(mSoftKeyboard);
	mSoftKeyboard->setPosition(mEngine.getWorldWidth() / 2.0f - mSoftKeyboard->getWidth()/2.0f, mEngine.getWorldHeight() / 2.0f - mSoftKeyboard->getHeight()/2.0f);
	mSoftKeyboard->setKeyPressFunction([this, ef](const std::wstring& character, ds::ui::SoftKeyboardDefs::KeyType keyType){
		if(keyType == ds::ui::SoftKeyboardDefs::kEnter){
			mSoftKeyboard->resetCurrentText();
		}
		mEngine.getNotifier().notify(KeyPressedEvent(mSoftKeyboard->getCurrentText()));
		//ef->setCurrentText(sk->getCurrentText());
		ef->keyPressed(character, keyType);
	});

	rootSprite.addChildPtr(new TextTest(mGlobals));
}

void KeyboardExample::update() {
	ds::App::update();

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

void KeyboardExample::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

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
	} else if(event.getCode() == KeyEvent::KEY_g) {
		if(mSoftKeyboard) {
			auto sks = mSoftKeyboard->getSoftKeyboardSettings();
			sks.mGraphicKeys = !sks.mGraphicKeys;
			mSoftKeyboard->setSoftKeyboardSettings(sks);
		}
	} else if(event.getCode() == KeyEvent::KEY_h) {
		if(mSoftKeyboard) {
			auto sks = mSoftKeyboard->getSoftKeyboardSettings();
			if(sks.mGraphicRoundedCornerRadius > 0.0f) {
				sks.mGraphicRoundedCornerRadius = 0.0f;
			} else {
				sks.mGraphicRoundedCornerRadius = 15.0f;
			}
			mSoftKeyboard->setSoftKeyboardSettings(sks);
		}
	} else if(event.getCode() == KeyEvent::KEY_c) {
		if(mSoftKeyboard) {
			auto sks = mSoftKeyboard->getSoftKeyboardSettings();
			sks.mKeyUpColor = ci::Color::white();
			sks.mKeyDownColor = ci::Color(0.5f, 0.5f, 0.5f);
			if(sks.mGraphicType == ds::ui::SoftKeyboardSettings::kCircularBorder) {
				sks.mGraphicType = ds::ui::SoftKeyboardSettings::kBorder;
			} else {
				sks.mGraphicType = ds::ui::SoftKeyboardSettings::kCircularBorder;
			}
			mSoftKeyboard->setSoftKeyboardSettings(sks);
		}
	} else if(event.getCode() == KeyEvent::KEY_v) {
		if(mSoftKeyboard) {
			auto sks = mSoftKeyboard->getSoftKeyboardSettings();
			sks.mKeyDownColor = ci::Color::white();
			sks.mKeyUpColor = ci::Color(0.25f, 0.25f, 0.25f);
			if(sks.mGraphicType == ds::ui::SoftKeyboardSettings::kCircularSolid) {
				sks.mGraphicType = ds::ui::SoftKeyboardSettings::kSolid;
			} else {
				sks.mGraphicType = ds::ui::SoftKeyboardSettings::kCircularSolid;
			}
			mSoftKeyboard->setSoftKeyboardSettings(sks);
		}
	} else if(event.getCode() == KeyEvent::KEY_b) {
		if(mSoftKeyboard) {
			auto sks = mSoftKeyboard->getSoftKeyboardSettings();
			if(sks.mGraphicBorderWidth > 1.0f) {
				sks.mGraphicBorderWidth = 1.0f;
			} else {
				sks.mGraphicBorderWidth = 5.0f;
			}
			mSoftKeyboard->setSoftKeyboardSettings(sks);
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