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
	
	bool					   getApplyParticles() override;
	ds::model::ContentModelRef getPinboard() override;
	std::vector<ds::model::ContentModelRef> getValidPinboards() override;
	ds::model::ContentModelRef getAnnotationFolder() override;


	// Inherited via WafflesHelper these are from WaffleHelper's base class ContentHelper
	std::string getCompositeKeyForPlatform() override;
	ds::model::ContentModelRef getRecordByUid(std::string uid) override;
	ds::Resource			   getBackgroundForPlatform() override;
	ds::model::ContentModelRef getPresentation() override;
	ds::model::ContentModelRef getAmbientPlaylist() override;
	std::string				   getInitialPresentationUid() override;
	std::vector<ds::model::ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) override;
	std::vector<ds::model::ContentModelRef> getContentForPlatform() override;
	std::vector<ds::Resource>				findMediaResources() override;

  private:
	ds::model::BaseContentHelper mBaseContentHelper;
};
}