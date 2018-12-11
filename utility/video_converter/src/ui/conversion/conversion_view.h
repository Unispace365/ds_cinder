#pragma once
#ifndef _VIDEO_CONVERTER_APP_UI_STORY_STORY_VIEW_H_
#define _VIDEO_CONVERTER_APP_UI_STORY_STORY_VIEW_H_

#include <ds/ui/layout/smart_layout.h>

namespace downstream {
class ConversionItem;

/**
* \class downstream::ConversionView
*			A view that shows what files are being converted
*/
class ConversionView : public ds::ui::SmartLayout  {
public:
	ConversionView(ds::ui::SpriteEngine& eng);


	std::vector<ConversionItem*>		mItems;

	void								startConversion();
	void								relayout();
};

} // namespace downstream

#endif

