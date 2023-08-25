#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace downstream {


/**
* \class downstream::StoryController
*			loads the tree and shows it in a single-pane sorta style
*/
class StoryController : public ds::ui::SmartLayout {
public:
	StoryController(ds::ui::SpriteEngine& eng);


	void goBack();
	void setData(ds::model::ContentModelRef parent);

	bool mInitialized = false;

	void buildBreadCrumbs(std::vector<std::string>& theNames, ds::model::ContentModelRef currentModel);
	void showInfoPage(ds::model::ContentModelRef theInfo);
};

} // namespace downstream


