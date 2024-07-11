#pragma once

#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/sprite/text.h>

namespace waffles {
class LauncherPane;

/**
 * \class ds::LauncherContent
 *			The content for the Launcher Viewer (i.e. all the buttons and stuff)
 */
class LauncherCmsContent : public ds::ui::SmartLayout {
  public:
	LauncherCmsContent(ds::ui::SpriteEngine& g);
	void showPresentations();
	void showRecents();

  protected:
	ds::ui::LayoutButton* setupTypeButton(const std::string& buttonName);
	void				  buttonClicked(ds::ui::LayoutButton* sb, const std::string& panelTitle);

	void getContentOfType(const std::string& mediaType, ds::model::ContentModelRef parentNode,
						  std::vector<ds::model::ContentModelRef>& outVec);

	void getContentOfResourceType(const int& mediaType, ds::model::ContentModelRef parentNode,
								  std::vector<ds::model::ContentModelRef>& outVec);

	void getContentOfType(const std::string& mediaType, std::vector<ds::model::ContentModelRef>& parentNodes,
						  std::vector<ds::model::ContentModelRef>& outVec);

	void getContentOfResourceType(const int& mediaType, std::vector<ds::model::ContentModelRef>& parentNodes,
								  std::vector<ds::model::ContentModelRef>& outVec);

	void sortMediaAlpha(std::vector<ds::model::ContentModelRef>& theMedia);
	void setPaneData(const std::string& mediaType, const std::string& paneTitle);
	void setPaneData(const int resourceType, const std::string& paneTitle);
	void setPaneData(std::vector<int> resourceTypes, const std::string& paneTitle);

	void setPaneData(std::vector<ds::model::ContentModelRef> newData, const bool canBack,
					 ds::model::ContentModelRef backModel = ds::model::ContentModelRef());

	ds::ui::Sprite* mPaneHolder = nullptr;

	std::vector<ds::ui::LayoutButton*> mTypeButtons;
	ds::ui::LayoutButton*			   mRecentButton		= nullptr;
	ds::ui::LayoutButton*			   mFoldersButton		= nullptr;
	ds::ui::LayoutButton*			   mPinboardsButton		= nullptr;
	ds::ui::LayoutButton*			   mVideoButton			= nullptr;
	ds::ui::LayoutButton*			   mStreamButton		= nullptr;
	ds::ui::LayoutButton*			   mImageButton			= nullptr;
	ds::ui::LayoutButton*			   mPdfButton			= nullptr;
	ds::ui::LayoutButton*			   mWebButton			= nullptr;
	ds::ui::LayoutButton*			   mPresetsButton		= nullptr;
	ds::ui::LayoutButton*			   mPresentationsButton = nullptr;

	ds::ui::LayoutButton* mBackButton		   = nullptr;
	ds::ui::Text*		  mBackTitle		   = nullptr;
	ds::ui::Text*		  mBackHighTitle	   = nullptr;
	ds::ui::Text*		  mCurrentTitle		   = nullptr;
	LauncherPane*		  mCurrentLauncherPane = nullptr;

	ds::model::ContentModelRef mBackMediaRef;
};

} // namespace waffles
