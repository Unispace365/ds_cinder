#pragma once
#ifndef LAYOUT_BUILDER_UI_SPRITE_INSPECTOR
#define LAYOUT_BUILDER_UI_SPRITE_INSPECTOR


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>


#include <ds/ui/layout/layout_sprite.h>

namespace layout_builder {

class Globals;
class TreeItem;

/**
* \class layout_builder::SpriteInspector
*			View the properties of a single sprite
*/
class SpriteInspector : public ds::ui::Sprite  {
public:
	SpriteInspector(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	void								animateOn();
	void								animateOff();

	void								inspectSprite(ds::ui::Sprite*);
	void								layout();

	void								addSpriteProperty(const std::wstring& propertyName, const ds::BitMask& propertyValue);
	void								addSpriteProperty(const std::wstring& propertyName, const bool propertyValue);
	void								addSpriteProperty(const std::wstring& propertyName, const ci::Vec2f propertyValue);
	void								addSpriteProperty(const std::wstring& propertyName, const ci::Vec3f propertyValue);
	void								addSpriteProperty(const std::wstring& propertyName, const ci::Color propertyValue);
	void								addSpriteProperty(const std::wstring& propertyName, const float propertyValue);
	void								addSpriteProperty(const std::wstring& propertyName, const int propertyValue);
	void								addSpriteProperty(const std::wstring& propertyName, const std::wstring& propertyValue);
	void								addSpritePropertyLayoutSizeMode(const std::wstring& propertyName, const int propertyValue);
	void								addSpritePropertyLayoutVAlign(const std::wstring& propertyName, const int propertyValue);
	void								addSpritePropertyLayoutHAlign(const std::wstring& propertyName, const int propertyValue);
	void								addSpritePropertyLayoutType(const std::wstring& propertyName, const ds::ui::LayoutSprite::LayoutType& propertyValue);
	void								addSpritePropertyLayoutShrink(const std::wstring& propertyName, const ds::ui::LayoutSprite::ShrinkType& propertyValue);
	void								addSpritePropertyBlend(const std::wstring& propertyName, const ds::ui::BlendMode& blendMode);
	void								doAddSpriteProperty(const std::wstring& propertyName, const std::wstring& propertyValue);

	Globals&							mGlobals;

	ds::EventClient						mEventClient;

	ds::ui::Sprite*						mLinkedSprite;
	ds::ui::LayoutSprite*				mLayout;
	std::vector<TreeItem*>				mTreeItems;
	TreeItem*							mCurrentInputTreeItem;

	void								setInputField(TreeItem*);
	void								clearProperties();
};

} // namespace layout_builder

#endif
