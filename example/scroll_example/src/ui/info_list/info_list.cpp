#include "info_list.h"


#include "app/globals.h"
#include "ui/info_list/info_list_item.h"

namespace example{

InfoList::InfoList(Globals& g)
	: ScrollList(g.mEngine)
	, mGlobals(g)
{


	setSize(600.0f, 600.0f);
	enable(false);


	setItemTappedCallback([this](ds::ui::Sprite* bs, const ci::Vec3f& cent){
		InfoListItem* rpi = dynamic_cast<InfoListItem*>(bs);
		if(rpi && mInfoCallback){
			mInfoCallback(rpi->getInfo(), cent);
		}
	});

	setCreateItemCallback([this]()->ds::ui::Sprite* {
		const float itemSize = mGlobals.getSettingsLayout().getFloat("info_list:item:height", 0, 100.0f);
		return new InfoListItem(mGlobals, getWidth(), itemSize);
	});

	setDataCallback([this](ds::ui::Sprite* bs, int dbId){
		InfoListItem* rpi = dynamic_cast<InfoListItem*>(bs);
		if(rpi){
			rpi->setInfo(mInfoMap[dbId]);
		}
	});

	setAnimateOnCallback([this](ds::ui::Sprite* bs, const float delay){
		InfoListItem* rpi = dynamic_cast<InfoListItem*>(bs);
		if(rpi){
			rpi->animateOn(delay);
		}
	});

	setStateChangeCallback([this](ds::ui::Sprite* bs, const bool highlighted){
		InfoListItem* rpi = dynamic_cast<InfoListItem*>(bs);
		if(rpi){
			rpi->setState(highlighted);
		}
	});


	const float itemSize = mGlobals.getSettingsLayout().getFloat("info_list:item:height", 0, 100.0f);
	const float padding = mGlobals.getSettingsLayout().getFloat("info_list:item:pad", 0, 20.0f);

	mStartPositionX = 0.0f;
	mStartPositionY = padding;
	mIncrementAmount = itemSize + padding;
	mFillFromTop = true;
}

// Another part of the app has given us new data
// We map that data with id's to story models
void InfoList::setInfo(const std::vector<ds::model::StoryRef>& infoList){

	mInfoMap.clear();

	// The vector of ints is how ScrollList keeps track of items.
	// Db Id's are unique.
	// We keep a map of the ids in this class so we can match up the UI with the content when new items scroll into view
	std::vector<int> productIds;
	for(auto it = infoList.begin(); it < infoList.end(); ++it){
		productIds.push_back((*it).getId());
		mInfoMap[(*it).getId()] = (*it);
	}

	setContent(productIds);
}



void InfoList::setInfoItemCallback(const std::function<void(const ds::model::StoryRef infothing, const ci::Vec3f possy)>& func){
	mInfoCallback = func;
}

}
