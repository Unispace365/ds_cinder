#pragma once
#ifndef DS_UI_LAYOUT_SMART_LAYOUT_SPRITE
#define DS_UI_LAYOUT_SMART_LAYOUT_SPRITE


#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace ds {
namespace ui {

/**
* \class ds::ui::SmartLayout
*        SmartLayout combines a layout sprite with XML importing.
*/
class SmartLayout : public ds::ui::LayoutSprite {
  public:

	/// Automatically loads the xmlLayout file (at %APP%/data/layouts/xmlLayoutFile) and runs the layout upon creation
	SmartLayout(ds::ui::SpriteEngine& engine, std::string xmlLayoutFile);

	/// If this smart layout has a child with this name
	bool hasSprite(const std::string& spriteName);

	/// Gets the sprite with the name specified. Returns nullptr if it doesn't exist.
	ds::ui::Sprite* getSprite(const std::string& spriteName);

	/// Gets the sprite with the name specified typecast to the template type. Returns nullptr if it doesn't exist or is a different type.
	template <typename T>
	T* getSprite(const std::string& spriteName);

	/// Adds newChild to the sprite with spriteName
	void addSpriteChild(const std::string spriteName, ds::ui::Sprite* newChild);

	/// Calls the lambda callback for the event type from theEvent::WHAT() 
	void listenToEvents(size_t type, std::function<void(const ds::Event&)> callback);

	/// Sets the wide text for a Text sprite with a name of spriteName
	void setSpriteText(const std::string& spriteName, const std::wstring& theText);
	/// Sets the text for a Text sprite with a name of spriteName
	void setSpriteText(const std::string& spriteName, const std::string& theText);
	/// Configurations the Text sprite with a name of spriteName
	void setSpriteFont(const std::string& spriteName, const std::string& textConfig);

	/// Set the image file for an Image sprite with name of spriteName. Image path is relative to %APP%/data/images/
	void setSpriteImage(const std::string& spriteName, const std::string& imagePath);
	/// Set the image resource for an Image sprite with name of spriteName.
	void setSpriteImage(const std::string& spriteName, ds::Resource);

	/// Set the tap function on a sprite named spriteName
	void setSpriteTapFn(const std::string& spriteName,
						const std::function<void(ds::ui::Sprite*, const ci::vec3&)>& tapCallback);

	// Build => Run Animations on children
	// this->addAnimation(name, duration, delay).size(elementName, to, delay=0).opacity(elementName, to,
	// delay=0).finishFn(callback);
	// this->addAnimationScript(name, file-or-string)
	// this->runAnimation(name)

	// Run Animations on children immediately
	// this->animate(duration, delay)
	// this->animate(file-or-string)

  protected:
	using sMap			= std::map<std::string, ds::ui::Sprite*>;
	using eventCallback = std::function<void(const ds::Event&)>;
	using eventMap		= std::unordered_map<size_t, eventCallback>;

	void onAppEvent(const ds::Event&);

	std::string		mLayoutFile;
	ds::EventClient mEventClient;
	sMap			mSpriteMap;
	eventMap		mEventCallbacks;
};

}  // namespace ui
}  // namespace ds

#endif
