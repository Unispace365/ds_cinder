#pragma once

#include <memory>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/sprite.h>


namespace waffles {

/**
 * \class waffles::wafflesHelper
 *			The background layer for interactive playlists and templates
 */

class WafflesHelper;

typedef std::shared_ptr<WafflesHelper> WafflesHelperPtr;

class WafflesHelperFactory {
  public:
	WafflesHelperFactory()=delete;

	template<class T> static void InitHelper(ds::ui::SpriteEngine& eng) {
		if (mDefault) {
			DS_LOG_WARNING("WafflesHelperFactory::InitHelper() called more than once");
		}
		mDefault = std::make_shared<T>(eng);
	}
	template<class T=WafflesHelper> static std::shared_ptr<T> getDefault() { return std::dynamic_pointer_cast<T>(mDefault); }

  private:
	static WafflesHelperPtr mDefault;
	
};



class WafflesHelper {
  public:
	  struct PlaylistFilter {
		enum class FilterMode { All, PlatformFallback, PlatformOverride };
		std::string eventTypeKey;
		std::string	eventPropertyName;
		std::string platformPropertyName;
		std::string playlistTypeKey;
		FilterMode filterMode = FilterMode::PlatformFallback;
	};
	WafflesHelper(ds::ui::SpriteEngine& eng)
	  : mEngine(eng){};
	virtual std::string						getCompositeKeyForPlatform()	= 0;
	virtual ds::model::ContentModelRef		getRecordByUid(std::string uid)				= 0;
	virtual ds::Resource					getBackgroundForPlatform() = 0;
	virtual bool							getApplyParticles() = 0;
	virtual ds::model::ContentModelRef		getPresentation()=0; //getInteractivePlaylist
	virtual ds::model::ContentModelRef		getAmbientPlaylist()=0;
	virtual std::string						getInitialPresentationUid()=0;
	virtual ds::model::ContentModelRef				getPinboard()= 0;
	virtual ds::model::ContentModelRef				getAnnotationFolder()			   = 0;
	virtual std::vector<ds::model::ContentModelRef>	getFilteredPlaylists(const PlaylistFilter& filter)			   = 0;
	virtual std::vector<ds::model::ContentModelRef> getContentForPlatform()=0; //getAssets
	virtual std::vector<ds::model::ContentModelRef> getValidPinboards()=0;
	virtual std::vector<ds::Resource>				findMediaResources()=0;

  protected:
	ds::ui::SpriteEngine& mEngine;
};

} // namespace waffles
