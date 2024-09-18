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
		std::string eventTypeKey; //what type of event has the playlist
		std::string	eventPropertyName; //what is the property name on the event
		std::string platformPropertyName; // what is the property name on the platform
		std::string playlistTypeKey;	  // what type of playlist
		FilterMode filterMode = FilterMode::PlatformFallback;
	};
	  static const std::string DEFAULTCATEGORY;
	  static const std::string ANYCATEGORY;
	  static const std::string WAFFLESCATEGORY; 
	  static const std::string PRESENTATIONCATEGORY;
	  static const std::string AMBIENTCATEGORY;
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
	virtual bool									isValidFolder(ds::model::ContentModelRef model,std::string category=DEFAULTCATEGORY) = 0;
	virtual bool									isValidMedia(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY)  = 0;
	virtual bool									isValidPlaylist(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY)		= 0;
	virtual std::string								getMediaPropertyKey(ds::model::ContentModelRef model,std::string category = DEFAULTCATEGORY) = 0;
  protected:
	ds::ui::SpriteEngine& mEngine;
};



} // namespace waffles
