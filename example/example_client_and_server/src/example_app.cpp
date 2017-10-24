#include "example_app.h"

#include <Poco/Path.h>
#include <cinder/Rand.h>

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/cfg/settings.h>
#include <ds/data/resource_list.h>
#include <ds/ui/touch/touch_info.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/text.h>

#include "custom_sprite.h"
namespace mv {

CsApp::CsApp()
	: mToggleSprite(nullptr)
	, mTouchDebug(mEngine)
{

	mEngine.editFonts().registerFont("Arial", "arial");

	mEngine.installSprite([](ds::BlobRegistry& r){ds::ui::CustomSprite::installAsServer(r); },
						  [](ds::BlobRegistry& r){ds::ui::CustomSprite::installAsClient(r); });
}

void CsApp::setupServer() {
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();

	// Example image sprite from a hardcoded filename.
	imgSprite = new ds::ui::Image(mEngine, "%APP%/data/lorem_kicksum.png");
	imgSprite->setScale(0.25f, 0.25f);
	imgSprite->enable(true);
	imgSprite->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
	imgSprite->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		imgSprite->sendToFront();
	});
	rootSprite.addChild(*imgSprite);

	ds::ui::Image* kittySprite = new ds::ui::Image(mEngine, "%APP%/data/kitty.jpg");
	kittySprite->setCenter(0.5f, 0.5f);
	kittySprite->setPosition(mEngine.getWorldWidth() / 2.0f, 200);
	kittySprite->enable(true);
	kittySprite->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
	kittySprite->setProcessTouchCallback([this, kittySprite](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		kittySprite->sendToFront();
	});
	rootSprite.addChildPtr(kittySprite);

	// Example sprite
	ds::ui::Sprite *child = new ds::ui::Sprite(mEngine, 100.0f, 100.0f);
	child->setPosition(getWindowWidth() / 4.0f, getWindowHeight() / 4.0f);
	child->setCenter(0.5f, 0.5f);
	child->setColor(1.0f, 1.0f, 0.0f);
	child->setTransparent(false);
	child->enable(true);
	child->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
	child->setProcessTouchCallback([this, child](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
		child->sendToFront();
	});
	rootSprite.addChild(*child);

	// Example text sprite
	mTexty = new ds::ui::Text(mEngine);
	mTexty->setFont("arial", 16.0f);
	mTexty->setText("<span weight='bold'>Welcome to the server client setup in ds_cinder!</span> \t\t This text should be displaying across both the <span style='italic'>client</span> and the server!");
	mTexty->setPosition(getWindowWidth() * 0.25f, getWindowHeight() * 0.75f);
	mTexty->getWidth();
	mTexty->setText("Well hello there");
	mTexty->enable(true);
	mTexty->setResizeLimit(10000.0f);
//	mTexty->setLeading(1.2f);
	mTexty->setAlignment(ds::ui::Alignment::kLeft);
	rootSprite.addChildPtr(mTexty);

	mCustomNetSprite = new ds::ui::CustomSprite(mEngine);
	mCustomNetSprite->setPosition(mEngine.getWorldWidth()/2.0f - 150.0f, 300.0f);
	mCustomNetSprite->setSize(300.0f, 300.0f);
	mCustomNetSprite->setColor(ci::Color(0.7f, 0.2f, 0.1f));
	rootSprite.addChildPtr(mCustomNetSprite);

	recreateText();
}


void CsApp::update(){
	ds::App::update();
}

void CsApp::recreateText(){
	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	if(mTexty){
		mTexty->release();
		mTexty = new ds::ui::Text(mEngine);
		mTexty->setFont("arial", 16.0f);
		std::stringstream ss;
		ss << "<span weight='bold'>Hey</span> there" << ci::randFloat() << " " << std::endl << ci::randFloat() << " " << ci::randFloat();
		mTexty->setText(ss.str());
		mTexty->setText("Well hello there");
		mTexty->setText("<span weight='bold'>Welcome to the server client setup in ds_cinder!</span> \t\t This text should be displaying across both the <span style='italic'>client</span> and the server!");
		mTexty->setPosition(getWindowWidth() * 0.25f, getWindowHeight() * 0.75f);
		mTexty->getWidth();
		mTexty->enable(true);
		mTexty->setResizeLimit(ci::randFloat(10.0f, 300.0f));
		//	mTexty->setLeading(1.2f);
		mTexty->setAlignment(ds::ui::Alignment::kLeft);
		rootSprite.addChild(*mTexty);

	}
	mTexty->callAfterDelay([this]{recreateText(); }, 5.0f);

	if(imgSprite){
		imgSprite->release();
		imgSprite = new ds::ui::Image(mEngine, "%APP%/data/lorem_kicksum.png");
		imgSprite->setScale(0.25f, 0.25f);
		imgSprite->enable(true);
		imgSprite->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
		imgSprite->setProcessTouchCallback([this](ds::ui::Sprite* bs, const ds::ui::TouchInfo& ti){
			imgSprite->sendToFront();
		});
		rootSprite.addChild(*imgSprite);
	}

}

void CsApp::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void CsApp::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void CsApp::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void CsApp::keyDown(ci::app::KeyEvent e) {
	ds::App::keyDown(e);

	const int		code = e.getCode();
	if(code == ci::app::KeyEvent::KEY_z) {
		if(!mToggleSprite) {
			mToggleSprite = newToggleSprite();
		} else {
			mToggleSprite->release();
			mToggleSprite = nullptr;
		}
	} else if(code == ci::app::KeyEvent::KEY_c){
		if(mCustomNetSprite){
			mCustomNetSprite->setNumberOfSegments(ci::randInt(3, 10));
		}
	} else if(code == ci::app::KeyEvent::KEY_i){
		if(mCustomNetSprite){
			mCustomNetSprite->setNumberOfInstances(ci::randInt(1, 50));
		}
	}
}

ds::ui::Sprite* CsApp::newToggleSprite() const {
	ds::ui::Sprite&		root(mEngine.getRootSprite());
	ds::ui::Sprite*		s = new ds::ui::Sprite(mEngine);
	if(!s) return nullptr;
	s->setTransparent(false);
	s->setColor(0.8f, 0.12f, 0.21f);
	s->enable(true);
	s->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);
	s->setPosition(root.getWidth()*0.2f, root.getHeight()*0.2f);
	s->setSize(root.getWidth()*0.4f, root.getHeight()*0.4f);
	root.addChild(*s);

	// Add a little child so delete is REALLY tested
	ds::ui::Sprite*		child = new ds::ui::Sprite(mEngine);
	if(child) {
		child->setTransparent(false);
		child->setColorA(ci::ColorA(0.0f, 0.0f, 0.0f, 0.24f));
		child->setSize(s->getWidth()*0.4f, s->getHeight()*0.4f);
		s->addChild(*child);
	}
	return s;
}

}
// This line tells Cinder to actually create the application
CINDER_APP(mv::CsApp, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))