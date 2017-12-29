#ifndef _MEDIAVIEWER_APP_H_
#define _MEDIAVIEWER_APP_H_

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <ds/app/app.h>
#include <ds/ui/sprite/image.h>
#include "ds/touch/touch_debug.h"
#include <ds/app/event_client.h>

#include "custom_sprite.h"

namespace mv {

class CsApp : public ds::App {
public:
	CsApp();

	virtual void			mouseDown(ci::app::MouseEvent e);
	virtual void			mouseDrag(ci::app::MouseEvent e);
	virtual void			mouseUp(ci::app::MouseEvent e);

	virtual void			onKeyDown(ci::app::KeyEvent event) override;
	void					setupServer();
	void					update();

	void					recreateText();

	//virtual void			fileDrop(ci::app::FileDropEvent event);

private:


	ds::ui::Sprite*			newToggleSprite() const;
	ds::TouchDebug			mTouchDebug;
	ds::ui::Text*			mTexty;
	ds::ui::Image*			imgSprite;
	ds::ui::Sprite*			mToggleSprite;

	ds::ui::CustomSprite*	mCustomNetSprite;
};

} // !namespace mv

#endif // !_MEDIAVIEWER_APP_H_
