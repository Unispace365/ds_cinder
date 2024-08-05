#pragma once

#include <memory>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/sprite.h>

namespace ds::model {

/**
 * \class ds::wafflesHelper
 *			The background layer for interactive playlists and templates
 */

class ContentHelper;

typedef std::shared_ptr<ContentHelper> ContentHelperPtr;

class ContentHelperFactory {
  public:
	ContentHelperFactory()=delete;

	template<class T> static void InitHelper(ds::ui::SpriteEngine& eng) {
		if (mDefault) {
			DS_LOG_WARNING("ContentHelperFactory::InitHelper() called more than once");
		}
		mDefault = std::make_shared<T>(eng);
	}
	template<class T=ContentHelper> static std::shared_ptr<T> getDefault() { return std::dynamic_pointer_cast<T>(mDefault); }

  private:
	static ContentHelperPtr mDefault;
	
};



class ContentHelper {
  public:
	  struct PlaylistFilter {
		enum class FilterMode { All, PlatformFallback, PlatformOverride };
		std::string eventTypeKey;
		std::string	eventPropertyName;
		std::string platformPropertyName;
		std::string playlistTypeKey;
		FilterMode filterMode = FilterMode::PlatformFallback;
	};
	ContentHelper(ds::ui::SpriteEngine& eng)
	  : mEngine(eng){};
	virtual std::string						getCompositeKeyForPlatform()	= 0;
	virtual ds::model::ContentModelRef		getRecordByUid(std::string uid)				= 0;
	virtual ds::Resource					getBackgroundForPlatform() = 0;

	virtual ds::model::ContentModelRef		getPresentation()=0; //getInteractivePlaylist
	virtual ds::model::ContentModelRef		getAmbientPlaylist()=0;
	virtual std::string						getInitialPresentationUid()=0;

	virtual std::vector<ds::model::ContentModelRef>	getFilteredPlaylists(const PlaylistFilter& filter)			   = 0;
	virtual std::vector<ds::model::ContentModelRef> getContentForPlatform()=0; //getAssets

	virtual std::vector<ds::Resource>				findMediaResources()=0;

  protected:
	ds::ui::SpriteEngine& mEngine;
};

} // namespace waffles
