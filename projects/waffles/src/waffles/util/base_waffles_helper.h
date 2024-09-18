#pragma once
#include <waffles/util/waffles_helper.h>
#include <ds/content/base_content_helper.h>

namespace waffles {

using namespace ds::model;

class BaseWafflesHelper : public WafflesHelper {
  public:
	BaseWafflesHelper(ds::ui::SpriteEngine& eng);
	~BaseWafflesHelper();
	   
	// Inherited via WafflesHelper
	
	virtual bool					   getApplyParticles() override;
	virtual ds::model::ContentModelRef		getPinboard() override;
	virtual std::vector<ds::model::ContentModelRef> getValidPinboards() override;
	virtual ds::model::ContentModelRef				getAnnotationFolder() override;


	// Inherited via WafflesHelper these are from WaffleHelper's base class ContentHelper
	virtual std::string getCompositeKeyForPlatform() override;
	virtual ds::model::ContentModelRef getRecordByUid(std::string uid) override;
	virtual  ds::Resource			   getBackgroundForPlatform() override;
	virtual int						   getBackgroundPdfPage() override;
	virtual ds::model::ContentModelRef getPresentation() override;
	virtual ds::model::ContentModelRef getAmbientPlaylist() override;
	virtual std::string				   getInitialPresentationUid() override;
	virtual std::vector<ds::model::ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) override;
	virtual std::vector<ds::model::ContentModelRef> getContentForPlatform() override;
	virtual std::vector<ds::Resource>				findMediaResources() override;

	virtual bool isValidFolder(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	virtual bool isValidMedia(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	virtual bool isValidPlaylist(ds::model::ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	virtual std::string getMediaPropertyKey(ds::model::ContentModelRef model,
											std::string				   category = DEFAULTCATEGORY) override;

  protected:
	std::unordered_map<std::string, std::vector<std::string>> mAcceptableFolders;
	std::unordered_map<std::string, std::vector<std::string>> mAcceptableMedia;
	//std::unordered_map<std::string, std::vector<std::string>> mAcceptablePresentations;
	std::unordered_map<std::string, std::vector<std::string>> mAcceptablePlaylists;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mMediaProps;

  protected:
	virtual void loadIntegration();
  private:
	ds::model::BaseContentHelper mBaseContentHelper;
	std::string mEventFieldKey;
	std::string mPlatformFieldKey;
	std::vector<std::string> mAnnotationFolderKeys;
	bool mUseRoot;
};
}