#pragma once

#include "ds/content/content_helper.h"

namespace ds::model {

class BaseContentHelper : public ContentHelper {
  public:
	BaseContentHelper(ui::SpriteEngine& eng);

	//! Returns the platform key defined in the application settings.
	std::string getPlatformKey() const override;
	//! Returns the platform type for the current platform, which is a human-readable string defined in the CMS schema.
	std::string getPlatformType() const override { return getPlatformType(getPlatformKey()); }
	//! Returns the platform type for the specified \a platformKey, which is a human-readable string defined in the CMS
	//! schema.
	std::string getPlatformType(const std::string& platformKey) const override;
	//! Returns all the events scheduled for the current platform, already sorted in order of importance.
	const std::vector<ContentModelRef>& getPlatformEvents() const override {
		return getPlatformEvents(getPlatformKey());
	}
	//! Returns all the events scheduled for this platform, already sorted in order of importance.
	const std::vector<ContentModelRef>& getPlatformEvents(const std::string& platformKey) const override;
	//! Returns the composite key defined in the waffles settings.
	std::string getCompositeKeyForPlatform() override;
	//! Returns the content model for the specified \a uid, or an empty model if not found.
	ContentModelRef getRecordByUid(const std::string& uid) const override;
	//! Returns the content model for the "current_content" stored in the engine, if applicable.
	ContentModelRef getCurrentContent() const override;
	//! Returns the default background resource, or an empty resource if not defined.
	Resource getBackgroundForPlatform() override;
	//! Returns the default presentation playlist found in the content, or an empty model if not found.
	ContentModelRef getPresentation() override;
	//! Returns the first ambient playlist found in the content, or an empty model if not found.
	ContentModelRef getAmbientPlaylist() override;
	//! Returns the uid of the default presentation playlist found in the content, or an empty string if not found.
	std::string getInitialPresentationUid() override;

	std::vector<ContentModelRef> getContentForPlatform() override;
	std::vector<Resource>		 findMediaResources() override;
	std::vector<ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) override;

	bool isValidFolder(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	bool isValidMedia(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	bool isValidPlaylist(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	bool isValidStreamSource(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	bool isValidStream(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;

	std::string getMediaPropertyKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;

	std::vector<ContentModelRef> getStreamSources(const std::string& category = DEFAULTCATEGORY) override;
	ContentModelRef				 getStreamSourceForStream(ContentModelRef	 stream,
														  const std::string& category = DEFAULTCATEGORY) override;
	std::string getStreamMatchKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	std::string getStreamSourceAddressKey(ContentModelRef	 model,
										  const std::string& category = DEFAULTCATEGORY) override;
	std::string getStreamSourceTypeKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;

  protected:
	const std::vector<std::string>&						getAcceptableFolders(const std::string& category) const;
	const std::vector<std::string>&						getAcceptableMedia(const std::string& category) const;
	const std::vector<std::string>&						getAcceptablePlaylists(const std::string& category) const;
	const std::vector<std::string>&						getAcceptableStreamSources(const std::string& category) const;
	const std::vector<std::string>&						getAcceptableStreams(const std::string& category) const;
	const std::unordered_map<std::string, std::string>& getStreamSourceAddressProps(const std::string& category) const;
	const std::unordered_map<std::string, std::string>& getStreamSourceTypeProps(const std::string& category) const;
	const std::unordered_map<std::string, std::string>& getStreamMatchProp(const std::string& category) const;
	const std::unordered_map<std::string, std::string>& getMediaProps(const std::string& category) const;

	//! Use lazy initialization to load the acceptable folders, media, playlists, and stream sources.
	void initializeAcceptableMedia() const;
	//! Use lazy initialization to load the acceptable folders, media, playlists, and stream sources.
	void initializeStreamSources() const;

  private:
	mutable std::unordered_map<std::string, std::vector<std::string>>					  mAcceptableFolders;
	mutable std::unordered_map<std::string, std::vector<std::string>>					  mAcceptableMedia;
	mutable std::unordered_map<std::string, std::vector<std::string>>					  mAcceptablePlaylists;
	mutable std::unordered_map<std::string, std::vector<std::string>>					  mAcceptableStreamSources;
	mutable std::unordered_map<std::string, std::vector<std::string>>					  mAcceptableStreams;
	mutable std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamSourceAddressProps;
	mutable std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamSourceTypeProps;
	mutable std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamMatchProp;
	mutable std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mMediaProps;
};

} // namespace ds::model