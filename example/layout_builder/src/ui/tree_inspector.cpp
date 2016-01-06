#include "tree_inspector.h"

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

TreeInspector::TreeInspector(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mGlobals(g)
	, mEventClient(g.mEngine.getNotifier(), [this](const ds::Event *m){ if(m) this->onAppEvent(*m); })
	, mLayout(nullptr)
{
	setPosition(800.0f, 200.0f);
	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION);

}

void TreeInspector::onAppEvent(const ds::Event& in_e){
	if(in_e.mWhat == InspectTreeRequest::WHAT()){
		const InspectTreeRequest& e((const InspectTreeRequest&)in_e);
		inspectTree(e.mTree);
	} else if(in_e.mWhat == RefreshLayoutRequest::WHAT() || in_e.mWhat == LoadLayoutRequest::WHAT()){
		//clearTree();
	}

}

void TreeInspector::clearTree(){
	if(mLayout){
		mLayout->release();
		mLayout = nullptr;
	}

	mTreeItems.clear();
}

void TreeInspector::inspectTree(ds::ui::Sprite* sp) {
	clearTree();

	// todo: make this an xml
	mLayout = new ds::ui::LayoutSprite(mEngine);
	mLayout->setSpriteName(L"This layout");
	mLayout->setLayoutType(ds::ui::LayoutSprite::kLayoutVFlow);
	mLayout->setSpacing(5.0f);
	mLayout->setShrinkToChildren(ds::ui::LayoutSprite::kShrinkBoth);
	mLayout->setTransparent(false);
	mLayout->setColor(ci::Color(0.2f, 0.2f, 0.2f));
	addChildPtr(mLayout);
	
	treeParseRecursive(sp, 0);

	layout();
	animateOn();
}

void TreeInspector::treeParseRecursive(ds::ui::Sprite* sp, const int indent){
	addTreeItem(sp, indent);
	sp->forEachChild([this, indent](ds::ui::Sprite& chill){
		treeParseRecursive(&chill, indent + 1);
	}, false);
}

void TreeInspector::addTreeItem(ds::ui::Sprite* sprid, const int indent){
	if(!mLayout) return;

	TreeItem* treeItem = new TreeItem(mGlobals, sprid);
	treeItem->mLayoutFudge.x = (float)(indent)* 15.0f;
	mLayout->addChildPtr(treeItem);
	mTreeItems.push_back(treeItem);

	
}

void TreeInspector::layout(){
	if(mLayout){
		mLayout->runLayout();
		setSize(mLayout->getWidth(), mLayout->getHeight());
	}
}


void TreeInspector::animateOn(){
	if(mLayout){
		mLayout->tweenAnimateOn(true, 0.0f, 0.1f);
	}
}

void TreeInspector::animateOff(){
	tweenOpacity(0.0f, mGlobals.getSettingsLayout().getFloat("story_view:anim_time", 0, 0.35f), 0.0f, ci::EaseNone(), [this]{hide(); });
}



} // namespace layout_builder
