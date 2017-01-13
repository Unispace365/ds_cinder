#ifndef _MEDIA_TESTER_APP_H_
#define _MEDIA_TESTER_APP_H_

#include <cinder/app/App.h>
#include <ds/ui/sprite/text.h>
#include <ds/app/app.h>
#include <ds/ui/sprite/panoramic_video.h>

namespace test {
class AllData;
//class dlpr::view::DroneVideoSprite;
class video_360 : public ds::App {
public:
	video_360();

	virtual void			fileDrop(ci::app::FileDropEvent event);
	virtual void			keyDown(ci::app::KeyEvent event);
	void					setupServer();
	void					update();
private:
	typedef ds::App			inherited;

	void					loadMedia(const std::string& newMedia);
	void					fitSpriteInArea(ci::Rectf area, ds::ui::Sprite* spriddy);
	ds::ui::PanoramicVideo* mPanoramicVideo;
	ds::ui::Sprite*			mOverlay;
	ds::ui::Text*			mLabelText;
	ds::ui::Sprite*			mHeader;
};

} // !namespace test

#endif // !_MEDIA_TESTER_APP_H_