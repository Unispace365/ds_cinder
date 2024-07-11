#pragma once

#include "waffles/viewers/base_element.h"

#include <ds/thread/serial_runnable.h>
#include <ds/ui/button/layout_button.h>
#include <ds/ui/layout/smart_layout.h>

#include "waffles/query/search_query.h"

namespace waffles {

/**
 * \class ds::SearchViewer
 *			A viewer panel that lets you search
 */
class SearchViewer : public BaseElement {
  public:
	SearchViewer(ds::ui::SpriteEngine& g, const std::string& searchType);

  protected:
	void		 startSearch(const std::string& searchStr);
	void		 onSearchResults(SearchQuery& q);
	virtual void onLayout();

	void listFolder(ds::model::ContentModelRef theFolder);

	ds::EventClient		 mEventClient;
	ds::ui::SmartLayout* mPrimaryLayout;

	ds::SerialRunnable<SearchQuery> mSearchQuery;
	std::string						mSearchText;

	void setupFilterButton(const std::string& buttonName, const std::string& mediaType, const int resourceFilter);
	void handleFilterButton(ds::ui::LayoutButton*, std::string mediaType, const int resourceFilter);


	ds::ui::LayoutButton* mSelectedFilterButton;
	std::string			  mCurrentFilter;
	int					  mResourceFilter;

	std::vector<ds::ui::LayoutButton*> mFilterButtons;
};

} // namespace waffles
