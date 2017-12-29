#ifndef _MEDIA_TESTER_APP_H_
#define _MEDIA_TESTER_APP_H_

#include <cinder/app/App.h>
#include <ds/ui/sprite/text.h>
#include <ds/app/app.h>

namespace test {
class AllData;

class media_tester : public ds::App {
public:
	media_tester();

	virtual void		fileDrop(ci::app::FileDropEvent event);
	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();
private:
	typedef ds::App		inherited;

	void				loadMedia(const std::string& newMedia);
	void				fitSpriteInArea(ci::Rectf area, ds::ui::Sprite* spriddy);
	ds::ui::Sprite*		mMedia;
	ds::ui::Text*		mLabelText;
	ds::ui::Sprite*		mHeader;
	bool				mIsVideo;
};

} // !namespace test

#endif // !_MEDIA_TESTER_APP_H_

