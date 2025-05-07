#pragma once

#include <memory>

namespace ds::model {

//! Abstract base class for content helpers. Provides implementation for some common methods.
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

	//! Returns the platform model for the current platform.
	virtual ContentModelRef getPlatformModel() const { return getRecordByUid(getPlatformKey()); }
	//! Returns the platform model for the specified \a platformKey.
	virtual ContentModelRef getPlatformModel(const std::string& platformKey) const {
		return getRecordByUid(platformKey);
	}

	//! Returns the platform key defined in the application settings.
	virtual std::string getPlatformKey() const = 0;
	//! Returns the platform type for the current platform, which is a human-readable string defined in the CMS schema.
	virtual std::string getPlatformType() const = 0;
	//! Returns the platform type for the specified \a platformKey, which is a human-readable string defined in the CMS
	//! schema.
	virtual std::string getPlatformType(const std::string& platformKey) const = 0;
	//! Returns all the events scheduled for the current platform, already sorted in order of importance.
	virtual const std::vector<ContentModelRef>& getPlatformEvents() const = 0;
	//! Returns all the events scheduled for the specified \a platformKey, already sorted in order of importance.
	virtual const std::vector<ContentModelRef>& getPlatformEvents(const std::string& platformKey) const = 0;
	//! Returns the composite key defined in the waffles settings.
	virtual std::string getCompositeKeyForPlatform() = 0;
	//! Returns the content model for the specified \a uid, or an empty model if not found.
	virtual ContentModelRef getRecordByUid(const std::string& uid) const = 0;
	//! Returns the content model for the "current_content" stored in the engine, if applicable.
	virtual ContentModelRef getCurrentContent() const = 0;
	//! Returns the default background resource, or an empty resource if not defined.
	virtual Resource getBackgroundForPlatform() = 0;
	//! Returns the default presentation playlist found in the content, or an empty model if not found.
	virtual ContentModelRef getPresentation()			= 0;
	//! Returns the first ambient playlist found in the content, or an empty model if not found.
	virtual ContentModelRef getAmbientPlaylist()		= 0;
	//! Returns the uid of the default presentation playlist found in the content, or an empty string if not found.
	virtual std::string		getInitialPresentationUid() = 0;

	virtual std::vector<ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) = 0;
	virtual std::vector<ContentModelRef> getContentForPlatform()							= 0; // getAssets
	virtual std::vector<ContentModelRef> getStreamSources(const std::string& category = DEFAULTCATEGORY)		 = 0;
	virtual ContentModelRef				 getStreamSourceForStream(ContentModelRef	 stream,
																  const std::string& category = DEFAULTCATEGORY) = 0;

	virtual std::vector<Resource> findMediaResources() = 0;

	virtual bool isValidFolder(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)	   = 0;
	virtual bool isValidMedia(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)		   = 0;
	virtual bool isValidStreamSource(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) = 0;
	virtual bool isValidStream(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)	   = 0;
	virtual bool isValidPlaylist(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)	   = 0;

	virtual std::string getMediaPropertyKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) = 0;
	virtual std::string getStreamMatchKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY)	  = 0;
	virtual std::string getStreamSourceAddressKey(ContentModelRef	 model,
												  const std::string& category = DEFAULTCATEGORY)				  = 0;
	virtual std::string getStreamSourceTypeKey(ContentModelRef	  model,
											   const std::string& category = DEFAULTCATEGORY)					  = 0;

	virtual std::vector<ContentModelRef> getRecordsOfType(const std::vector<ContentModelRef>& records,
														  const std::string&				  type);

	virtual std::vector<ContentProperty> findAllProperties(const std::vector<ContentModelRef>& records,
														   const std::string&				   propertyName);

  protected:
	static void getRecordsByUid(const std::vector<ContentModelRef>& records, const std::string& uid,
								std::vector<ContentModelRef>& result);
	static void getRecordsByType(const std::vector<ContentModelRef>& records, const std::string& type,
								 std::vector<ContentModelRef>& result);
	static void getPropertyByName(const std::vector<ContentModelRef>& records, const std::string& propertyName,
								  std::vector<ContentProperty>& result);

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
