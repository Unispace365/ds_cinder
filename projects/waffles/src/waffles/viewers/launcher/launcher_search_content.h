#pragma once

#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/smart_layout.h>
#include <ds/ui/sprite/text.h>
#include <ds/thread/serial_runnable.h>

#include "waffles/query/search_query.h"

namespace waffles {
class LauncherPane;

/**
 * \class ds::LauncherContent
 *			The content for the Launcher Viewer (i.e. all the buttons and stuff)
 */
class LauncherSearchContent : public ds::ui::SmartLayout {
  public:
	LauncherSearchContent(ds::ui::SpriteEngine& g);

  protected:
	void		 startSearch(const std::string& searchStr);
	void		 onSearchResults(SearchQuery& q);
	virtual void onLayout();

	void listFolder(ds::model::ContentModelRef theFolder);

	void setupFilterButton(const std::string& buttonName, const std::string& mediaType, const int resourceFilter);
	void handleFilterButton(ds::ui::LayoutButton*, std::string mediaType, const int resourceFilter);

	ds::SerialRunnable<SearchQuery> mSearchQuery;
	std::string						mSearchText;

	ds::ui::LayoutButton*			   mSelectedFilterButton;
	std::string						   mCurrentFilter;
	int								   mResourceFilter;
	std::vector<ds::ui::LayoutButton*> mFilterButtons;
};

} // namespace waffles
