#pragma once
#include <waffles/util/waffles_helper.h>

namespace waffles {
class BaseWafflesHelper : public WafflesHelper {
  public:
	BaseWafflesHelper(ds::ui::SpriteEngine& eng);
	~BaseWafflesHelper();
	   
	// Inherited via WafflesHelper
	std::string getCompositeKeyForPlatform() override;
	ds::model::ContentModelRef getRecordByUid(std::string uid) override;
	ds::Resource			   getBackgroundForPlatform() override;
	bool					   getApplyParticles() override;

	// Inherited via WafflesHelper
	ds::model::ContentModelRef getPresentation() override;
	ds::model::ContentModelRef getAmbientPlaylist() override;
	std::string				   getInitialPresentationUid() override;
	ds::model::ContentModelRef getPinboard() override;
	std::vector<ds::model::ContentModelRef> getContentForPlatform() override;
	std::vector<ds::model::ContentModelRef> getValidPinboards() override;
	std::vector<ds::Resource>				findMediaResources() override;

	// Inherited via WafflesHelper
	std::vector<ds::model::ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) override;

	// Inherited via WafflesHelper
	ds::model::ContentModelRef getAnnotationFolder() override;
};
}