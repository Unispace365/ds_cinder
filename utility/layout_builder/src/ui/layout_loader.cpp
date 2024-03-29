#include "layout_loader.h"

#include <Poco/LocalDateTime.h>

#include <cinder/DataTarget.h>

#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/shader/sprite_shader.h>
#include <ds/util/string_util.h>
#include <ds/debug/logger.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"

#include <ds/ui/interface_xml/interface_xml_importer.h>

namespace layout_builder {

LayoutLoader::LayoutLoader(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mLayout(nullptr)
{
	
	setPosition(200.0f, 200.0f);
}

void LayoutLoader::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == RefreshLayoutRequest::WHAT()){
		ds::ui::SpriteShader::clearShaderCache();
		loadLayout(mLayoutLocation);
	} else if(in_e.mWhat == LayoutLayoutRequest::WHAT()){
		layout();
	} else if(in_e.mWhat == LoadLayoutRequest::WHAT()){
		const LoadLayoutRequest& e((const LoadLayoutRequest&)in_e);
		loadLayout(e.mLocation);
	} else if(in_e.mWhat == AnimateLayoutRequest::WHAT()){
		animateOn();
	} else if(in_e.mWhat == SaveLayoutRequest::WHAT()){
		saveLayout();
	} else if(in_e.mWhat == CreateSpriteRequest::WHAT()){
		const CreateSpriteRequest& e((const CreateSpriteRequest&)in_e);
		if(!e.mTypeName.empty()){
			addASprite(e.mTypeName);
		}
	} else if(in_e.mWhat == DeleteSpriteRequest::WHAT()){
		const DeleteSpriteRequest& e((const DeleteSpriteRequest&)in_e);
		ds::ui::Sprite* killah = e.mSpriteToDelete;
		if(killah){
			killah->release();
			layout();
			mEngine.getNotifier().notify(InspectTreeRequest(mLayout));
		}
	}
	
}

void LayoutLoader::loadLayout(const std::string& layoutLocation) {
	if(mLayout){
		mLayout->release();
		mLayout = nullptr;
	}

	mLayoutLocation = layoutLocation;
	if(mLayoutLocation.empty()) return;

	mLayout = new ds::ui::LayoutSprite(mEngine);
	mLayout->setSpriteName(ds::wstr_from_utf8(layoutLocation));
	mLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutNone);
	addChildPtr(mLayout);

	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(mLayout, ds::Environment::expand(mLayoutLocation), spriteMap);


	mEngine.getNotifier().notify(InspectTreeRequest(mLayout));

	layout();
	animateOn();
}

void LayoutLoader::layout(){
	if(mLayout){
		mLayout->runLayout();
		mLayout->clearAnimateOnTargets(true);
	}
}

void LayoutLoader::animateOn(){
	if(mLayout){
		mLayout->tweenAnimateOn(true, 0.0f, 0.1f);
	}
}

void LayoutLoader::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}

void LayoutLoader::saveLayout(){
	if(!mLayout){
		return;
	}
	ci::XmlTree xmlRoot = ci::XmlTree::createDoc();
	ci::XmlTree interfaceXml = ci::XmlTree("interface", "");

	auto sprids = mLayout->getChildren();
	for(auto it = sprids.begin(); it < sprids.end(); ++it){
		auto newXml = ds::ui::XmlImporter::createXmlFromSprite(*(*it));
		interfaceXml.push_back(newXml);
	//	buildXmlRecursive((*it), interfaceXml);
	}

	xmlRoot.push_back(interfaceXml);
	xmlRoot.write(ci::DataTargetPath::createRef(ds::Environment::expand("%APP%/data/layouts/test_save.xml")), false);
}

void LayoutLoader::buildXmlRecursive(ds::ui::Sprite* sp, ci::XmlTree& tree){
	ci::XmlTree newXml = ci::XmlTree("sprite", "");
	newXml.setAttribute("name", ds::utf8_from_wstr(sp->getSpriteName()));

	auto sprids = sp->getChildren();
	for(auto it = sprids.begin(); it < sprids.end(); ++it){
		buildXmlRecursive((*it), newXml);
	}
	tree.push_back(newXml);
}

void LayoutLoader::addASprite(const std::string& typeName){
	if(!mLayout) return;
	ds::ui::Sprite* spridy = ds::ui::XmlImporter::createSpriteByType(mEngine, typeName);
	if(spridy){
		mLayout->addChildPtr(spridy);
		layout();
		mEngine.getNotifier().notify(InspectTreeRequest(mLayout));
	}
}


} // namespace layout_builder
