#pragma once
#ifndef LAYOUT_BUILDER_UI_LAYOUT_LOADER
#define LAYOUT_BUILDER_UI_LAYOUT_LOADER

#include <cinder/Xml.h>

#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/layout/layout_sprite.h>

namespace layout_builder {

class Globals;

/**
* \class layout_builder::LayoutLoader
*			Load a layout
*/
class LayoutLoader : public ds::ui::Sprite  {
public:
	LayoutLoader(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	void								animateOn();
	void								animateOff();

	void								addASprite(const std::string& typeName);
	void								loadLayout(const std::string& layoutLocation);
	void								layout();
	void								saveLayout();
	void								buildXmlRecursive(ds::ui::Sprite* sp, ci::XmlTree& tree);
	Globals&							mGlobals;

	ds::EventClient						mEventClient;

	ds::ui::LayoutSprite*				mLayout;
	std::string							mLayoutLocation;


};

} // namespace layout_builder

#endif
