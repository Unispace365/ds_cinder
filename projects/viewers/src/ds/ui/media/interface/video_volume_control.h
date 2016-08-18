#pragma once
#ifndef DS_UI_MEDIA_VIDEO_INTERFACE_VIDEO_VOLUME_CONTROL
#define DS_UI_MEDIA_VIDEO_INTERFACE_VIDEO_VOLUME_CONTROL

#include "ds/ui/media/media_interface.h"

namespace ds {
namespace ui { 
class GstVideo;

/**
* \class ds::ui::VideoVolumeControl
*			Control audio volume for a video
*/
class VideoVolumeControl : public ds::ui::Sprite  {
public:
	VideoVolumeControl(ds::ui::SpriteEngine& eng, const float theSize = 50.0f, const float buttHeight = 25.0f, const ci::Color interfaceColor = ci::Color::white());

	void							linkVideo(ds::ui::GstVideo* linkedVideo);
	void							setVolume(const float v);

protected:
	virtual void					updateServer(const ds::UpdateParams& updateParams);

	ds::ui::GstVideo*				mLinkedVideo;

	std::vector<ds::ui::Sprite*>	mBars;
	float							mOffOpacity;


};

} // namespace ui
} // namespace ds

#endif
