#include "layout_builder_app.h"

#include <Poco/String.h>
#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/button/sprite_button.h>
#include <ds/ui/media/media_viewer.h>
#include <ds/ui/soft_keyboard/soft_keyboard_defs.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>

#include <cinder/Rand.h> 
#include <cinder/app/RendererGl.h>

#include "app/app_defs.h"
#include "app/globals.h"

#include "events/app_events.h"

#include "ui/layout_loader.h"
#include "ui/tree_inspector.h"
#include "ui/sprite_inspector.h"
#include "ui/overall_controller.h"
#include "ui/sprite_creator.h"


namespace layout_builder {

layout_builder::layout_builder()
	: inherited(ds::RootList()
								.ortho() 
								.pickColor() ) 
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mTouchDebug(mEngine)
	, mController(nullptr)
	, mInputField(nullptr)
	, mEventClient(mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
{

	enableCommonKeystrokes(true);
}

void layout_builder::setupServer(){

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");

	/*fonts in use */
	mEngine.loadSettings("FONTS", "fonts.xml");
	mEngine.editFonts().clear();
	mEngine.getSettings("FONTS").forEachTextKey([this](const std::string& key){
		mEngine.editFonts().registerFont(ds::Environment::expand(mEngine.getSettings("FONTS").getText(key)), key);
	});

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

	mController = nullptr;

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));

	rootSprite.addChildPtr(new LayoutLoader(mGlobals));
	rootSprite.addChildPtr(new TreeInspector(mGlobals));
	rootSprite.addChildPtr(new SpriteInspector(mGlobals));
	rootSprite.addChildPtr(new SpriteCreator(mGlobals));
	rootSprite.addChildPtr(new OverallController(mGlobals));

	
}

void layout_builder::update() {
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

void layout_builder::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

	if(mInputField){
		std::wstring inOutKey;
		std::wstring curText = mInputField->getText();
		ds::ui::SoftKeyboardDefs::handleKeyPressGeneric(event, inOutKey, curText);
		mEngine.getNotifier().notify(InputFieldTextInput(mInputField, curText));
		return;
	}
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
	} else if(event.getCode() == KeyEvent::KEY_v){
		loadLayout(ds::Environment::getClipboard());
	}
}

void layout_builder::loadLayout(const std::string& location){
	mEngine.getNotifier().notify(LoadLayoutRequest(location));
}

void layout_builder::mouseDown(ci::app::MouseEvent e) {
	mEngine.getNotifier().notify(InputFieldCleared());
	mTouchDebug.mouseDown(e);
}

void layout_builder::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void layout_builder::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void layout_builder::mouseMove( ci::app::MouseEvent e ) {
	ci::vec3 p(e.getX(), e.getY(), 0.0f);
	mEngine.getNotifier().notify(MouseMoveEvent(p));
}

void layout_builder::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		loadLayout((*it).string());
		break;
	}
}

void layout_builder::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == InputFieldSetRequest::WHAT()){
		const InputFieldSetRequest& e((const InputFieldSetRequest&)in_e);
		mInputField = e.mInputField;
	} else if(in_e.mWhat == AppRestartRequest::WHAT()){
		// wait a frame to restart so everything can be handled okee dokee
		mEngine.getRootSprite().callAfterDelay([this]{
			setupServer();
		}, 0.01f);
	}
}

} // namespace layout_builder

// This line tells Cinder to actually create the application
CINDER_APP(layout_builder::layout_builder, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))
