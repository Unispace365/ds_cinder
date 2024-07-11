#pragma once

#include <ds/app/event_client.h>
#include <ds/content/content_model.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/sprite/text.h>

#include "waffles/query/directory_query.h"
#include <ds/thread/serial_runnable.h>
namespace waffles {
class LauncherPane;

/**
 * \class ds::LauncherLocalContent
 *			The content for the Launcher Viewer, though this is all stuff found only on the local HD
 */
class LauncherLocalContent : public ds::ui::SmartLayout {
  public:
	LauncherLocalContent(ds::ui::SpriteEngine& g);

	void selectDocuments();

  protected:
	ds::ui::LayoutButton* setupDriveButton(const std::string& buttonName, const std::string& directoryPath);
	void				  buttonClicked(ds::ui::LayoutButton* sb, const std::string& directoryPath);
	void				  onDirectoryQuery(DirectoryQuery& q);

	void requestPaneData(const std::string& panePath);
	void setPaneData(ds::model::ContentModelRef newMedia, const bool isError);

	ds::ui::LayoutSprite* mMainPanel;
	ds::ui::Sprite*		  mPaneHolder;

	ds::ui::Sprite*					   mDrivesHolder;
	std::vector<ds::ui::LayoutButton*> mDriveButtons;
	ds::ui::LayoutButton*			   mDocumentsButton;
	ds::ui::LayoutButton*			   mUserButton;

	ds::ui::LayoutButton* mBackButton;
	ds::ui::Text*		  mBackTitle;
	ds::ui::Text*		  mBackHighTitle;
	ds::ui::Text*		  mCurrentTitle;

	std::vector<ds::model::ContentModelRef> mHistory;

	LauncherPane* mCurrentLauncherPane;

	ds::SerialRunnable<DirectoryQuery> mDirectoryQuery;
};

} // namespace waffles
