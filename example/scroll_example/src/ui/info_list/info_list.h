#pragma once
#ifndef DS_UI_EXAMPLE_SCROLL_EXAMPLE_UI_INFO_LIST_INFO_LIST
#define DS_UI_EXAMPLE_SCROLL_EXAMPLE_UI_INFO_LIST_INFO_LIST

#include <ds/ui/scroll/scroll_list.h>
#include <map>
#include "model/generated/story_model.h"

namespace example
{
	class Globals;


	class InfoList : public ds::ui::ScrollList {
	public:
		InfoList(Globals& g);

		void						setInfo(const std::vector<ds::model::StoryRef>& infoList);


		void						setInfoItemCallback(const std::function<void(const ds::model::StoryRef infothing, const ci::Vec3f possy)> &func);

	private:
		Globals&													mGlobals;
		std::map<int, ds::model::StoryRef>							mInfoMap;
		std::function<void(const ds::model::StoryRef infothing, const ci::Vec3f possy)>	mInfoCallback;

	};

}

#endif