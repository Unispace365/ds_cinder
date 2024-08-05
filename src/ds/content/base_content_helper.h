#pragma once
#include "content_helper.h"

namespace ds::model {
class BaseContentHelper : public ContentHelper {
  public:
	BaseContentHelper(ds::ui::SpriteEngine& eng);
	~BaseContentHelper();
	   
	// Inherited via ContentHelper
	std::string getCompositeKeyForPlatform() override;
	ds::model::ContentModelRef getRecordByUid(std::string uid) override;
	ds::Resource			   getBackgroundForPlatform() override;
	ds::model::ContentModelRef getPresentation() override;
	ds::model::ContentModelRef getAmbientPlaylist() override;
	std::string				   getInitialPresentationUid() override;
	std::vector<ds::model::ContentModelRef> getContentForPlatform() override;
	std::vector<ds::Resource>				findMediaResources() override;
	std::vector<ds::model::ContentModelRef> getFilteredPlaylists(const PlaylistFilter& filter) override;

};
}