#pragma once
#ifndef _RESOURCE_PARSER_APP_UI_STORY_STORY_VIEW_H_
#define _RESOURCE_PARSER_APP_UI_STORY_STORY_VIEW_H_

#include <ds/ui/layout/smart_layout.h>
#include <Poco/File.h>

namespace downstream {

/**
* \class downstream::StoryView
*			A view that shows a single story. Disappears when idle starts, and reappears on user action
*/
class StoryView : public ds::ui::SmartLayout  {
public:
	StoryView(ds::ui::SpriteEngine& eng);

private:

	void								parseDirectoryRecursive(Poco::File theRootDir);
	void								animateOn();
	void								animateOff();

	void								setData();

	std::string							mDbUrl;
	int									mResourcesId;
	std::string							mResourcesRoot;

	int									mNumUpdated;
	int									mNumSkipped;
	int									mNumInvalid;
	int									mNumError;

	std::vector<std::string>			mCurrentResources;
};

} // namespace downstream

#endif

