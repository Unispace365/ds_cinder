#pragma once

#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/scroll/scroll_list.h>

namespace waffles {

/**
 * \class waffles::LauncherPane
 *				A list of items in the Launcher
 */
class LauncherPane : public ds::ui::SmartLayout {
  public:
	LauncherPane(ds::ui::SpriteEngine& g);

	void setData(std::vector<ds::model::ContentModelRef> mediaList);
	void animateOn();

	void setFolderBrowseCallback(std::function<void(ds::model::ContentModelRef)> func) { mFolderBrowseCallback = func; }

	bool getBackMediaModel(ds::model::ContentModelRef& outModel);

  protected:
	virtual void onSizeChanged();
	void		 layout();

	ds::ui::ScrollList*								mFileList;
	std::map<int, ds::model::ContentModelRef>		mInfoMap;
	std::function<void(ds::model::ContentModelRef)> mFolderBrowseCallback;
};

} // namespace waffles
