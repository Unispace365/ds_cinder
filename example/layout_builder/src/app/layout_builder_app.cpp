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
	, mController(nullptr)
	, mInputField(nullptr)
	, mEventClient(mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
{

}

void layout_builder::setupServer(){

	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");


	mGlobals.initialize();
	mQueryHandler.runInitialQueries();

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

void layout_builder::onKeyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;

	if(mInputField){
		std::wstring inOutKey;
		std::wstring curText = mInputField->getText();
		ds::ui::SoftKeyboardDefs::handleKeyPressGeneric(event, inOutKey, curText);
		mEngine.getNotifier().notify(InputFieldTextInput(mInputField, curText));
		return;
	} else if(event.getCode() == KeyEvent::KEY_v){
		loadLayout(ds::Environment::getClipboard());
	}
	// Reload current layout file with F5 key
	else if (event.getCode() == KeyEvent::KEY_F5)
		mEngine.getNotifier().notify(RefreshLayoutRequest());

}

void layout_builder::loadLayout(const std::string& location){
	mEngine.getNotifier().notify(LoadLayoutRequest(location));
}

void layout_builder::mouseDown(ci::app::MouseEvent e) {
	mEngine.getNotifier().notify(InputFieldCleared());
}

void layout_builder::mouseDrag(ci::app::MouseEvent e) {
}

void layout_builder::mouseUp(ci::app::MouseEvent e) {
}

void layout_builder::mouseMove( ci::app::MouseEvent e ) {
	inherited::mouseMove(e);
	auto alteredMouseEvent = mEngine.alteredMouseEvent(e);
	ci::vec3 p(alteredMouseEvent.getX(), alteredMouseEvent.getY(), 0.0f);
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
