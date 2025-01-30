#pragma once
#include "content_helper.h"

namespace ds::model {
class BaseContentHelper : public ContentHelper {
  public:
	BaseContentHelper(ds::ui::SpriteEngine& eng);
	~BaseContentHelper();
	   
	// Inherited via ContentHelper
	virtual std::string getCompositeKeyForPlatform() override;
	virtual ds::model::ContentModelRef getRecordByUid(std::string uid) override;
	virtual ds::Resource			   getBackgroundForPlatform() override;
	virtual ds::model::ContentModelRef getPresentation() override;
	virtual ds::model::ContentModelRef getAmbientPlaylist() override;
	virtual std::string				   getInitialPresentationUid() override;
	virtual std::vector<ds::model::ContentModelRef> getContentForPlatform() override;
	virtual std::vector<ds::Resource>				findMediaResources() override;
	virtual std::vector<ds::model::ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) override;

	virtual bool isValidFolder(ds::model::ContentModelRef model,std::string category=DEFAULTCATEGORY) override;
	virtual bool isValidMedia(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	virtual bool isValidPlaylist(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY) override;

	virtual std::string getMediaPropertyKey(ds::model::ContentModelRef model,std::string category= DEFAULTCATEGORY) override;

	virtual std::vector<ds::model::ContentModelRef> getStreamSources(std::string category = DEFAULTCATEGORY) override;
	virtual ds::model::ContentModelRef				getStreamSourceForStream(ds::model::ContentModelRef stream,
																	 std::string				category = DEFAULTCATEGORY) override;
	virtual bool isValidStreamSource(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	virtual bool isValidStream(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	virtual std::string	getStreamMatchKey(ds::model::ContentModelRef model, std::string category= DEFAULTCATEGORY) override;
	virtual std::string	getStreamSourceAddressKey(ds::model::ContentModelRef model, std::string category= DEFAULTCATEGORY) override;
	virtual std::string	getStreamSourceTypeKey(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY) override;

protected:
	std::unordered_map<std::string,std::vector<std::string>> mAcceptableFolders;
	std::unordered_map<std::string, std::vector<std::string>> mAcceptableMedia;
	//std::unordered_map<std::string, std::vector<std::string>> mAcceptablePresentations;
	std::unordered_map<std::string, std::vector<std::string>> mAcceptablePlaylists;
	std::unordered_map<std::string, std::vector<std::string>>					 mAcceptableStreamSources;
	std::unordered_map<std::string, std::vector<std::string>>					 mAcceptableStreams;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamSourceAddressProps;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamSourceTypeProps;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mStreamMatchProp;
	std::unordered_map<std::string, std::unordered_map<std::string,std::string>> mMediaProps;



};
}