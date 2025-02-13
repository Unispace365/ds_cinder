#pragma once
#include <ds/content/base_content_helper.h>
#include <ds/ui/media/media_interface.h>
#include <waffles/util/waffles_helper.h>
namespace waffles {

using namespace ds::model;

class BaseWafflesHelper : public WafflesHelper {
  public:
	BaseWafflesHelper(ds::ui::SpriteEngine& eng);
	~BaseWafflesHelper() override;

	BaseWafflesHelper(const BaseWafflesHelper&)			   = delete;
	BaseWafflesHelper(BaseWafflesHelper&&)				   = default;
	BaseWafflesHelper& operator=(const BaseWafflesHelper&) = delete;
	BaseWafflesHelper& operator=(BaseWafflesHelper&&)	   = delete;

	// Inherited via WafflesHelper

	bool						 getApplyParticles() override;
	ContentModelRef				 getPinboard() override;
	std::vector<ContentModelRef> getValidPinboards() override;
	ContentModelRef				 getAnnotationFolder() override;
	void						 setKeyboardStyle(ds::ui::SoftKeyboard* keeb) override;
	void						 setMediaInterfaceStyle(ds::ui::MediaInterface* interfacey) override;


	// Inherited via WafflesHelper these are from WaffleHelper's base class ContentHelper
	std::string					 getCompositeKeyForPlatform() override;
	ContentModelRef				 getRecordByUid(std::string uid) override;
	ds::Resource				 getBackgroundForPlatform() override;
	int							 getBackgroundPdfPage() override;
	ContentModelRef				 getPresentation() override;
	ContentModelRef				 getAmbientPlaylist() override;
	std::string					 getInitialPresentationUid() override;
	std::vector<ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) override;
	std::vector<ContentModelRef> getContentForPlatform() override;
	std::vector<ds::Resource>	 findMediaResources() override;

	bool				 isValidFolder(ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	bool				 isValidMedia(ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	bool				 isValidPlaylist(ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	std::string			 getMediaPropertyKey(ContentModelRef model, std::string category = DEFAULTCATEGORY) override;
	bool				 isValidForFilter(std::string filter, ContentModelRef model) override;
	void				 setLauncherCustomFilters(CustomFilters cf) override;
	const CustomFilters& getLauncherCustomFilters() override;
	void				 setLauncherCustomContent(CustomContent cc) override;
	const CustomContent& getLauncherCustomContent() override;

	std::vector<ContentModelRef> getStreamSources(std::string category) override;
	ContentModelRef				 getStreamSourceForStream(ContentModelRef stream, std::string category) override;
	bool						 isValidStreamSource(ContentModelRef model, std::string category) override;
	bool						 isValidStream(ContentModelRef model, std::string category) override;
	std::string					 getStreamMatchKey(ContentModelRef model, std::string category) override;
	std::string					 getStreamSourceAddressKey(ContentModelRef model, std::string category) override;
	std::string getStreamSourceTypeKey(ContentModelRef model, std::string category = DEFAULTCATEGORY) override;

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

  protected:
	virtual void loadIntegration();

  private:
	BaseContentHelper		 mBaseContentHelper;
	std::string				 mEventFieldKey;
	std::string				 mPlatformFieldKey;
	std::vector<std::string> mAnnotationFolderKeys;
	bool					 mUseRoot{};
	CustomFilters			 mLauncherCustomFilters;
	CustomContent			 mLauncherCustomContent;
};
} // namespace waffles
