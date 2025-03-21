#pragma once

#include "ds/content/content_helper.h"

namespace ds::model {

class BaseContentHelper : public ContentHelper {
  public:
	BaseContentHelper(ui::SpriteEngine& eng);

	// Inherited via ContentHelper
	std::string					 getCompositeKeyForPlatform() override;
	ContentModelRef				 getRecordByUid(const std::string& uid) override;
	Resource					 getBackgroundForPlatform() override;
	ContentModelRef				 getPresentation() override;
	ContentModelRef				 getAmbientPlaylist() override;
	std::string					 getInitialPresentationUid() override;
	std::vector<ContentModelRef> getContentForPlatform() override;
	std::vector<Resource>		 findMediaResources() override;
	std::vector<ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) override;
	std::vector<ContentModelRef> getRecordsOfType(const std::string& type) override;

	bool isValidFolder(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	bool isValidMedia(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	bool isValidPlaylist(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	bool isValidStreamSource(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	bool isValidStream(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;

	std::string getMediaPropertyKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;

	std::vector<ContentModelRef> getStreamSources(const std::string& category = DEFAULTCATEGORY) override;
	ContentModelRef				 getStreamSourceForStream(ContentModelRef stream, const std::string& category = DEFAULTCATEGORY) override;
	std::string					 getStreamMatchKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	std::string					 getStreamSourceAddressKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;
	std::string					 getStreamSourceTypeKey(ContentModelRef model, const std::string& category = DEFAULTCATEGORY) override;

  protected:
	std::unordered_map<std::string, std::vector<std::string>> mAcceptableFolders;
	std::unordered_map<std::string, std::vector<std::string>> mAcceptableMedia;
	// std::unordered_map<std::string, std::vector<std::string>> mAcceptablePresentations;
	std::unordered_map<std::string, std::vector<std::string>>					  mAcceptablePlaylists;
	std::unordered_map<std::string, std::vector<std::string>>					  mAcceptableStreamSources;
	std::unordered_map<std::string, std::vector<std::string>>					  mAcceptableStreams;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamSourceAddressProps;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamSourceTypeProps;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamMatchProp;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mMediaProps;
};

} // namespace ds::model