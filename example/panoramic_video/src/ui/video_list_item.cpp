#include "video_list_item.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include <ds/ui/interface_xml/interface_xml_importer.h>

#include "app/app_defs.h"
#include "app/globals.h"

namespace panoramic{
VideoListItem::VideoListItem(Globals& g, const float widthy, const float heighty)
	: ds::ui::Sprite(g.mEngine, widthy, heighty)
	, mGlobals(g)
	, mLabel(nullptr)
	, mBackground(nullptr)
	, mThumbnail(nullptr)
	, mLayout(nullptr)
{

	std::map<std::string, ds::ui::Sprite*>	spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/video_list_item.xml"), spriteMap);

	mLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["layout"]);
	mBackground = spriteMap["background_highlight"];
	mLabel = dynamic_cast<ds::ui::Text*>(spriteMap["title"]);
	mThumbnail = dynamic_cast<ds::ui::Image*>(spriteMap["thumbnail"]);

	if(!mThumbnail || !mLabel || !mBackground || !mLayout){
		DS_LOG_WARNING("Error creating related FolderBrowserItem item! Check XML! Or Somethingngngng!");
		return;
	}

	enable(true);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
}

void VideoListItem::setInfo(const ds::Resource info){
	mInfoModel = info;

	if(mLabel){
		std::string uppername = mInfoModel.getThumbnailFilePath();
		std::transform(uppername.begin(), uppername.end(), uppername.begin(), ::toupper);
		mLabel->setText(uppername);
	}

	if(mThumbnail){
		std::string thumbPath;
		if(mInfoModel.getType() == ds::Resource::IMAGE_TYPE){
			thumbPath = "%APP%/data/images/ui/Camera.png";
		} else if(mInfoModel.getType() == ds::Resource::PDF_TYPE){
			thumbPath = "%APP%/data/images/ui/PDF.png";
		} else if(mInfoModel.getType() == ds::Resource::VIDEO_TYPE){
			thumbPath = "%APP%/data/images/ui/Movie.png";
		} else if(mInfoModel.getType() == ds::Resource::WEB_TYPE){
			thumbPath = "%APP%/data/images/ui/Link.png";
		} else {
			thumbPath = "%APP%/data/images/ui/Source.png";
		}

		mThumbnail->setImageFile(thumbPath, ds::ui::Image::IMG_CACHE_F);
	}

	setState(0);
	layout();
}

ds::Resource VideoListItem::getInfo(){
	return mInfoModel;
}


void VideoListItem::animateOn(const float delay){
	if(mLayout){
		mLayout->tweenAnimateOn(true, delay, 0.05f);
	}
}

void VideoListItem::layout(){
	if(mLayout){
		mLayout->completeAllTweens(false, true);
		mLayout->setSize(getWidth(), getHeight());
		mLayout->runLayout();
	}
}

void VideoListItem::setState(const int buttonState){
	if(mBackground){
		ci::Color normalColor = mGlobals.getSettingsLayout().getColor("info_list:item:normal_color", 0, ci::Color::white());
		ci::Color highliColor = mGlobals.getSettingsLayout().getColor("info_list:item:highli_color", 0, ci::Color::white());
		if(buttonState == 1){
			mBackground->setColor(highliColor);
		} else {
			mBackground->setColor(normalColor);
		}
	}
}

}


