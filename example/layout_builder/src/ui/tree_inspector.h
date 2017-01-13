#pragma once
#ifndef LAYOUT_BUILDER_UI_TREE_INSPECTOR
#define LAYOUT_BUILDER_UI_TREE_INSPECTOR


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>


#include <ds/ui/layout/layout_sprite.h>

namespace layout_builder {

class Globals;
class TreeItem;

/**
* \class layout_builder::TreeInspector
*			View a sprite hierarchy
*/
class TreeInspector : public ds::ui::Sprite  {
public:
	TreeInspector(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	void								animateOn();
	void								animateOff();

	void								inspectTree(ds::ui::Sprite*);
	void								layout();

	Globals&							mGlobals;

	ds::EventClient						mEventClient;

	ds::ui::LayoutSprite*				mLayout;

	ds::ui::Sprite*						mTreeRoot;
	std::vector<TreeItem*>				mTreeItems;
	ds::ui::Sprite*						mHighlighter;

	void								treeParseRecursive(ds::ui::Sprite* sp, const int indent);
	void								addTreeItem(ds::ui::Sprite* sprid, const int indent);
	void								clearTree();
	void								handleMouseHover(const ci::vec3& mousePoint);
	void								highlightSprite( const ds::ui::Sprite* sprite );
};

} // namespace layout_builder

#endif
