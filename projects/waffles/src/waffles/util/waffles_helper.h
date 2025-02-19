#pragma once

#include <memory>

#include <ds/app/event_client.h>
#include <ds/content/content_helper.h>
#include <ds/ui/media/media_interface.h>
#include <ds/ui/soft_keyboard/soft_keyboard.h>
#include <ds/ui/sprite/sprite.h>
namespace waffles {

/**
 * \class waffles::wafflesHelper
 *			The background layer for interactive playlists and templates
 */

class WafflesHelper;

using WafflesHelperPtr = std::shared_ptr<WafflesHelper>;


class WafflesHelper : public ds::model::ContentHelper {
  public:
	WafflesHelper(ds::ui::SpriteEngine& eng)
	  : ds::model::ContentHelper(eng) {}

	using CustomFilters = std::unordered_map<std::string, std::function<bool(ds::model::ContentModelRef)>>;
	using CustomContent = std::unordered_map<std::string, std::function<void(ds::model::ContentModelRef, ci::vec3)>>;

	virtual bool									getApplyParticles()										   = 0;
	virtual ds::model::ContentModelRef				getPinboard()											   = 0;
	virtual ds::model::ContentModelRef				getAnnotationFolder()									   = 0;
	virtual std::vector<ds::model::ContentModelRef> getValidPinboards()										   = 0;
	virtual std::vector<ds::Resource>				findMediaResources()									   = 0;
	virtual int										getBackgroundPdfPage()									   = 0;
	virtual void									setKeyboardStyle(ds::ui::SoftKeyboard* keeb)			   = 0;
	virtual void									setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) = 0;

	virtual bool				 isValidForFilter(std::string filter, ds::model::ContentModelRef model) = 0;
	virtual void				 setLauncherCustomFilters(CustomFilters cf)								= 0;
	virtual const CustomFilters& getLauncherCustomFilters()												= 0;
	virtual void				 setLauncherCustomContent(CustomContent cc)								= 0;
	virtual const CustomContent& getLauncherCustomContent()												= 0;
};

} // namespace waffles
