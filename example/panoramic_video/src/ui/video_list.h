#pragma once
#ifndef DS_UI_VIEWERS_VIDEO_LIST
#define DS_UI_VIEWERS_VIDEO_LIST

#include <ds/ui/panel/base_panel.h>
#include <ds/ui/scroll/scroll_list.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/button/image_button.h>
#include <ds/app/event_client.h>

namespace panoramic {
class Globals;

class VideoList : public ds::ui::BasePanel  {
public:
	VideoList(Globals& g);

	void								animateOn();
	void								animateOff();

protected:
	void								onAppEvent(const ds::Event&);
	virtual void						onLayout();
	void								setData();

	Globals&							mGlobals;
	ds::ui::ScrollList*					mFileList;
	ds::ui::ScrollBar*					mScrollBar;
	ds::ui::ImageButton*				mCloseButton;

	std::map<int, ds::Resource>			mInfoMap;
	ds::EventClient						mEventClient;
};

} // namespace mv

#endif
