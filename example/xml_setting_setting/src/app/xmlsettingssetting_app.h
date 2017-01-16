#ifndef _XMLSETTINGSSETTING_APP_H_
#define _XMLSETTINGSSETTING_APP_H_

#include <cinder/app/App.h>
#include <ds/app/app.h>

#include "app/globals.h"

namespace setter {
class SettingsUi;

class XmlSettingsSetting : public ds::App {
public:
	XmlSettingsSetting();

	virtual void		keyDown(ci::app::KeyEvent event);
	void				setupServer();
	void				update();

private:
	typedef ds::App		inherited;

	// Data acquisition
	Globals				mGlobals;

	SettingsUi*			mSettings;
	ds::ui::Sprite*		mTestSprite;

	void				tweenTestSprite(ds::ui::Sprite* bs);
};

} // !namespace setter

#endif // !_XMLSETTINGSSETTING_APP_H_