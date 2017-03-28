#include "tree_inspector.h"

#include <Poco/LocalDateTime.h>

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/app/event_notifier.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>
#include <ds/ui/touch/drag_destination_info.h>

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
	, mTreeRoot(nullptr)
	, mHighlighter(nullptr)
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
	} else if(in_e.mWhat == InspectSpriteRequest::WHAT()){
		const InspectSpriteRequest& e((const InspectSpriteRequest&)in_e);

		for(auto it = mTreeItems.begin(); it < mTreeItems.end(); ++it){
			(*it)->getValueField()->setColor(ci::Color::white());
			if((*it)->getLinkedSprite() == e.mSprid){
				(*it)->getValueField()->setColor(ci::Color(0.8f, 0.4f, 0.4f));
			}
		}
	}
	else if (in_e.mWhat == MouseMoveEvent::WHAT() ) {
		const MouseMoveEvent& e( (const MouseMoveEvent&)in_e );
		handleMouseHover(e.mMousePoint);
	}
	else if (in_e.mWhat == ShowSpriteHighlightEvent::WHAT()) {
		const ShowSpriteHighlightEvent& e( (const ShowSpriteHighlightEvent&)in_e );
		highlightSprite( e.mSpriteToHighlight );
	}
}

void TreeInspector::highlightSprite( const ds::ui::Sprite* sprite ) {
	auto showHighlight = [&]( const ds::ui::Sprite* s ) {
		if (!mHighlighter) {
			mHighlighter = new ds::ui::Sprite( mEngine );
			mHighlighter->setTransparent( false );
			mHighlighter->setColorA( ci::ColorA::hexA( 0xAA6fa8dc ) );
			addChildPtr( mHighlighter );

			auto crosshairColor = ci::Color::hex( (0xf6b26b) );

			auto hCrosshair = new ds::ui::Sprite( mEngine, 40.0f, 3.0f );
			hCrosshair->setTransparent( false );
			hCrosshair->setColor( crosshairColor );
			mHighlighter->addChildPtr( hCrosshair );
			auto vCrosshair = new ds::ui::Sprite( mEngine, 3.0f, 40.0f );
			vCrosshair->setTransparent( false );
			vCrosshair->setColor( crosshairColor );
			mHighlighter->addChildPtr( vCrosshair );
		}

		mHighlighter->show();
		mHighlighter->setSize( s->getWidth(), s->getHeight() );
		auto pos = s->getGlobalPosition();
		mHighlighter->setPosition( globalToLocal( pos ) );
	};

	if (sprite) {
		showHighlight(sprite);
	}
	else if (mHighlighter) {
		mHighlighter->hide();
	}

}

void TreeInspector::handleMouseHover(const ci::vec3& mousePoint) {
	if (!this->contains( mousePoint ))
		return;

	static TreeItem* highlightedTreeItem = nullptr;
	TreeItem* newHighlightedTreeItem = nullptr;

	//DS_LOG_INFO( "Mouse moved inside TreeInspector" << mousePoint.x << ", " << mousePoint.y );
	for (TreeItem* treeItem : mTreeItems) {
		if (treeItem->contains(mousePoint)) {
			newHighlightedTreeItem = treeItem;
			break;
		}
	}

	if (newHighlightedTreeItem) {
		if (newHighlightedTreeItem != highlightedTreeItem) {
			highlightedTreeItem = newHighlightedTreeItem;
			highlightSprite(highlightedTreeItem->getLinkedSprite());
		}
	}
	else {
		highlightedTreeItem = nullptr;
		highlightSprite(nullptr);
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

	mTreeRoot = sp;

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

	mEngine.addToDragDestinationList(treeItem);

	treeItem->setDragDestinationCallback([this, treeItem](ds::ui::Sprite* sp, const ds::ui::DragDestinationInfo& di){
		if(di.mPhase == ds::ui::DragDestinationInfo::Entered){
			sp->setColor(ci::Color(0.6f, 0.2f, 0.2f));
		} else if(di.mPhase == ds::ui::DragDestinationInfo::Exited){
			sp->setColor(ci::Color(0.1f, 0.1f, 0.1f));
		}
		if(di.mPhase == ds::ui::DragDestinationInfo::Released){
			TreeItem* ti = dynamic_cast<TreeItem*>(di.mSprite);
			TreeItem* dest = dynamic_cast<TreeItem*>(sp);

			if(ti){
				ti->setColor(ci::Color(0.1f, 0.1f, 0.1f));
			}
			if(dest){
				dest->setColor(ci::Color(0.1f, 0.1f, 0.1f));
			}

			if(treeItem == dest) return;

			if(ti && dest){
				if(ti == dest) return;
				if(dest->getLinkedSprite() == ti->getLinkedSprite()){
					return;
				}

				// If the drag destination is already the parent of this sprite, then just move it to the end
				if(dest->getLinkedSprite() == ti->getLinkedSprite()->getParent()){
					ti->getLinkedSprite()->sendToFront();
				} else {
					dest->getLinkedSprite()->addChildPtr(ti->getLinkedSprite());
				}				

				callAfterDelay([this]{
					inspectTree(mTreeRoot);
					mEngine.getNotifier().notify(LayoutLayoutRequest());
				}, 0.01f);
			}
		}
	});

	treeItem->setProcessTouchCallback([this](ds::ui::Sprite* sp, const ds::ui::TouchInfo& ti){
		if(ti.mPhase == ds::ui::TouchInfo::Removed && ti.mNumberFingers == 0){
			callAfterDelay([this]{
				layout();
			}, 0.05f);
		}
	});

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
	tweenOpacity(0.0f, mGlobals.getAnimDur(), 0.0f, ci::EaseNone(), [this]{hide(); });
}



} // namespace layout_builder


