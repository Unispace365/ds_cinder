#pragma once

#include <memory>

#include <ds/app/event_client.h>
#include <ds/content/content_helper.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/ui/media/media_interface.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>
namespace waffles {

/**
 * \class waffles::wafflesHelper
 *			The background layer for interactive playlists and templates
 */

class WafflesHelper;

typedef std::shared_ptr<WafflesHelper> WafflesHelperPtr;


class WafflesHelper : public ds::model::ContentHelper {
  public:
	WafflesHelper(ds::ui::SpriteEngine& eng)
	  : ds::model::ContentHelper(eng){};

	virtual bool									getApplyParticles()	  = 0;
	virtual ds::model::ContentModelRef				getPinboard()		  = 0;
	virtual ds::model::ContentModelRef				getAnnotationFolder() = 0;
	virtual std::vector<ds::model::ContentModelRef> getValidPinboards()	  = 0;
	virtual std::vector<ds::Resource>				findMediaResources()  = 0;
	virtual int										getBackgroundPdfPage()= 0;
	virtual void									setKeyboardStyle(ds::ui::SoftKeyboard* keeb)=0;
	virtual void									setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey)=0;
	virtual bool									isValidForFilter(std::string filter, ds::model::ContentModelRef model) = 0;
	virtual void                                    setLauncherCustomFilters(std::unordered_map<std::string, std::function<bool(ds::model::ContentModelRef)>> cf) = 0;
	virtual std::unordered_map<std::string, std::function<bool(ds::model::ContentModelRef)>> getLauncherCustomFilters() = 0;
	virtual void                                    setLauncherCustomContent(std::unordered_map<std::string, std::function<void(ds::model::ContentModelRef)>> cc) = 0;
	virtual std::unordered_map<std::string, std::function<void(ds::model::ContentModelRef)>> getLauncherCustomContent() = 0;

};

} // namespace waffles
