#include "sprite_inspector.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include <typeinfo>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include "ui/tree_item.h"

namespace layout_builder {

SpriteInspector::SpriteInspector(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mLayout(nullptr)
	, mLinkedSprite(nullptr)
{

	setPosition(800.0f, 800.0f);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);

}

void SpriteInspector::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == InspectSpriteRequest::WHAT()){
		const InspectSpriteRequest& e((const InspectSpriteRequest&)in_e);
		inspectSprite(e.mSprid);
	} else if(in_e.mWhat == RefreshLayoutRequest::WHAT() || in_e.mWhat == LoadLayoutRequest::WHAT()){
		//clearTree();
	}

}
void SpriteInspector::inspectSprite(ds::ui::Sprite* sp) {
	if(mLayout){
		mLayout->release();
		mLayout = nullptr;
	}

	if(!sp) return;

	mLinkedSprite = sp;

	// todo: make this an xml
	mLayout = new ds::ui::LayoutSprite(mEngine);
	mLayout->setSpriteName(L"This layout");
	mLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	//mLayout->setSpacing(5.0f);
	mLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);
	mLayout->setTransparent(false);
	mLayout->setColor(ci::Color(0.2f, 0.2f, 0.2f));
	addChildPtr(mLayout);

	addSpriteProperty(L"Sprite Name:", sp->getSpriteName());
	addSpriteProperty(L"Class: ", ds::wstr_from_utf8(typeid(*sp).name()));
	addSpriteProperty(L"Size:", sp->getSize());
	addSpriteProperty(L"Color:", sp->getColor());
	addSpriteProperty(L"Opacity:", sp->getOpacity());
	addSpriteProperty(L"Position:", sp->getPosition());
	addSpriteProperty(L"Rotation:", sp->getRotation());
	addSpriteProperty(L"Scale:", sp->getScale());
	addSpriteProperty(L"Clipping:", sp->getClipping());
	addSpriteProperty(L"BlendMode:", sp->getBlendMode());
	addSpriteProperty(L"Enabled:", sp->isEnabled());
	addSpriteProperty(L"Transparent:", sp->getTransparent());
	addSpriteProperty(L"AnimateOn:", ds::wstr_from_utf8(sp->getAnimateOnScript()));
	addSpriteProperty(L"T Pad", sp->mLayoutTPad);
	addSpriteProperty(L"L Pad", sp->mLayoutLPad);
	addSpriteProperty(L"B Pad", sp->mLayoutBPad);
	addSpriteProperty(L"R Pad", sp->mLayoutRPad);

	ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(mLinkedSprite);
	if(texty){
		doAddSpriteProperty(L"-------------------------", L"-------------------------");
		addSpriteProperty(L"Text:", texty->getText());
		addSpriteProperty(L"Font:", ds::wstr_from_utf8(texty->getFontFileName()));
		addSpriteProperty(L"Font Size:", texty->getFontSize());
	}

	layout();
	animateOn();
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const std::wstring& propertyValue){
	doAddSpriteProperty(propertyName, propertyValue);
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const bool propertyValue){
	if(propertyValue){
		doAddSpriteProperty(propertyName, L"True");
	} else {
		doAddSpriteProperty(propertyName, L"False");
	}
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const ci::Color propertyValue){
	std::wstringstream wss;
	wss << L"r:" << propertyValue.r << L" g:" << propertyValue.g << L" b:" << propertyValue.b;
	doAddSpriteProperty(propertyName, wss.str());
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const ci::Vec3f propertyValue){
	std::wstringstream wss;
	wss << L"x:" << propertyValue.x << L" y:" << propertyValue.y << L" z:" << propertyValue.z;
	doAddSpriteProperty(propertyName, wss.str());
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const float propertyValue){
	doAddSpriteProperty(propertyName, ds::value_to_wstring(propertyValue));
}

void SpriteInspector::addSpriteProperty(const std::wstring& propertyName, const int propertyValue){
	doAddSpriteProperty(propertyName, ds::value_to_wstring(propertyValue));
}

void SpriteInspector::doAddSpriteProperty(const std::wstring& propertyName, const std::wstring& propertyValue){
	TreeItem* treeItem = new TreeItem(mGlobals, propertyName, propertyValue);
	mLayout->addChildPtr(treeItem);
}
void SpriteInspector::layout(){
	if(mLayout){
		mLayout->runLayout();
		setSize(mLayout->getWidth(), mLayout->getHeight() + 20.0f);
	}
}


void SpriteInspector::animateOn(){
	if(mLayout){
		mLayout->tweenAnimateOn(true, 0.0f, 0.1f);
	}
}

void SpriteInspector::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}



} // namespace layout_builder
