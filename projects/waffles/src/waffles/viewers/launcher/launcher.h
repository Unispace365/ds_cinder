#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/ui/button/image_button.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/smart_layout.h>

namespace waffles {
class LauncherCmsContent;
class LauncherLocalContent;
class LauncherSearchContent;
class ListItem;

/**
 * \class ds::Launcher
 *			Quickly find stuff to look at
 */
class Launcher : public BaseElement {
  public:
	Launcher(ds::ui::SpriteEngine& g, std::string eventChannel = "", bool hideClose = false);

	void updateMenuItems();
	void setupMenuItems();
	void showSearch();

  protected:
	void onLayout() override;
	void onCreationArgsSet() override;
	void onParentSet() override;


	void					   closePanel();
	ds::model::ContentModelRef buttonCfgFromString(const std::string& str) const;
	void					   updateItem(ds::ui::SmartLayout* item);
	static void				   updateSelection(ds::ui::Sprite* bs, const bool highlighted);
	void					   handleSelection();
	ds::ui::SmartLayout*	   createButton(ds::model::ContentModelRef item);
	void					   setButtonCallbacks(ds::ui::SmartLayout* assetBtn);
	void					   buttonTapHandler(ds::ui::Sprite* sp, const ci::vec3& pos);
	void					   panelButtonTapped(ds::ui::SmartLayout* button);
	void					   updatePanelContent(const ds::model::ContentModelRef& model);
	static bool				   unrepeatedContent(const ds::model::ContentModelRef& existing,
												 const ds::model::ContentModelRef& addition);
	bool					   restrictiveType(const ds::model::ContentModelRef& model) const;
	void					   updateRecent(const ds::model::ContentModelRef& model);
	void					   loadRecent();
	void					   saveRecent();
	void					   filterButtonDown(const std::string& type);
	void					   closeButtonPlacement();
	void					   setBackButtonFn(ds::ui::LayoutButton* button);
	std::string				   getRecentFilePath() const {
		   return ds::Environment::expand(mEngine.getEngineSettings().getString("resource_location", 0, "%LOCAL%/hpi/") +
													  +"waffles_recent.txt");
	}
	bool recentContains(const ds::model::ContentModelRef& model) {
		return std::find(mRecentFilterUids.begin(), mRecentFilterUids.end(), model.getPropertyString("uid")) !=
			   mRecentFilterUids.end();
	}
	std::vector<ds::model::ContentModelRef> recurseContent(const std::vector<ds::model::ContentModelRef>& content);
	void									updateBreadcrumbText();
	std::string								upperedFilterText();
	static void								filterItemIconHandle(ds::ui::SmartLayout* item);

	std::vector<ds::model::ContentModelRef> mFolderStack;

	std::vector<std::string> mRecentFilterUids;
	int						 mMaxRecentSize		 = 10;
	bool					 mRecentFileHandling = false;
	std::string				 mFilterSelected;

	std::unordered_map<std::string, ds::ui::SmartLayout*> mFilterButtons; // type name => button layout

	bool mFirstCloseButton	= true;
	bool mSecondCloseButton = false;

	bool mFirstFilterShove = true;

	ds::ui::SmartLayout* mPrimaryLayout = nullptr;

	std::vector<ds::model::ContentModelRef> mMenuItemsTop;
	std::vector<ds::model::ContentModelRef> mMenuItemsBottom;
	std::vector<ds::model::ContentModelRef> mMenuItemsScrolling;

	std::vector<ds::ui::SmartLayout*> mMainButtons;
	ds::model::ContentModelRef		  mSelectedMain;


	std::vector<ds::model::ContentModelRef> mPanelHistory;

	bool mNeedsTopRefresh		= false;
	bool mNeedsScrollingRefresh = false;
	bool mNeedsBottomRefresh	= false;

	bool  mPanelOpen			   = false;
	bool  mPanelTransitioning	   = false;
	float mWafflesScale			   = 1.0f;
	bool  mCloseOnViewerFullscreen = true;
};

} // namespace waffles
