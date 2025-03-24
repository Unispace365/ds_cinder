#pragma once

#include <memory>

namespace ds::model {

class ContentHelper {
  public:
	virtual ~ContentHelper() = default;

	ContentHelper(const ContentHelper&)			   = delete;
	ContentHelper(ContentHelper&&)				   = delete;
	ContentHelper& operator=(const ContentHelper&) = delete;
	ContentHelper& operator=(ContentHelper&&)	   = delete;

	struct PlaylistFilter {
		enum class FilterMode { All, PlatformFallback, PlatformOverride };
		std::string eventTypeKey;		  // what type of event has the playlist
		std::string eventPropertyName;	  // what is the property name on the event
		std::string platformPropertyName; // what is the property name on the platform
		std::string playlistTypeKey;	  // what type of playlist
		FilterMode	filterMode = FilterMode::PlatformFallback;
	};

	static const std::string DEFAULTCATEGORY;
	static const std::string ANYCATEGORY;
	static const std::string WAFFLESCATEGORY;
	static const std::string PRESENTATIONCATEGORY;
	static const std::string AMBIENTCATEGORY;

	ContentHelper(ui::SpriteEngine& eng)
	  : mEngine(eng) {}

	virtual std::string		getCompositeKeyForPlatform()		   = 0;
	virtual ContentModelRef getRecordByUid(const std::string& uid) = 0;
	virtual Resource		getBackgroundForPlatform()			   = 0;

	virtual ContentModelRef getPresentation()			= 0; // getInteractivePlaylist
	virtual ContentModelRef getAmbientPlaylist()		= 0;
	virtual std::string		getInitialPresentationUid() = 0;

	virtual std::vector<ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter)				 = 0;
	virtual std::vector<ContentModelRef> getContentForPlatform()										 = 0; // getAssets
	virtual std::vector<ContentModelRef> getStreamSources(const std::string& category = DEFAULTCATEGORY) = 0;
	virtual ContentModelRef				 getStreamSourceForStream(ContentModelRef stream, const std::string& category = DEFAULTCATEGORY) = 0;

	virtual std::vector<Resource> findMediaResources() = 0;

	virtual bool isValidFolder(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)	   = 0;
	virtual bool isValidMedia(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)		   = 0;
	virtual bool isValidStreamSource(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) = 0;
	virtual bool isValidStream(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)	   = 0;
	virtual bool isValidPlaylist(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)	   = 0;

	virtual std::string getMediaPropertyKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)		= 0;
	virtual std::string getStreamMatchKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)			= 0;
	virtual std::string getStreamSourceAddressKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) = 0;
	virtual std::string getStreamSourceTypeKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)	= 0;

	virtual std::vector<ContentModelRef> getRecordsOfType(const std::string& type) = 0;
	virtual std::vector<ContentModelRef> getRecordsOfType(const std::vector<ContentModelRef>& records,
														  const std::string&				  type) = 0;

  protected:
	static void getRecordsByUid(const std::vector<ContentModelRef>& records, const std::string& uid,
								std::vector<ContentModelRef>& result);
	static void getRecordsByType(const std::vector<ContentModelRef>& records, const std::string& type,
								 std::vector<ContentModelRef>& result);

	ui::SpriteEngine& mEngine;
};

using ContentHelperPtr = std::shared_ptr<ContentHelper>;

class ContentHelperFactory {
  public:
	ContentHelperFactory() = delete;

	template <class T>
	static void InitHelper(ui::SpriteEngine& eng) {
		if (mDefault) {
			DS_LOG_WARNING("ContentHelperFactory::InitHelper() called more than once");
		}
		mDefault = std::make_shared<T>(eng);
	}
	template <class T = ContentHelper>
	static std::shared_ptr<T> getDefault() {
		return std::dynamic_pointer_cast<T>(mDefault);
	}

  private:
	static ContentHelperPtr mDefault;
};


} // namespace ds::model
