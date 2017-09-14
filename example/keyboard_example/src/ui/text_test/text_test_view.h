#pragma once
#ifndef _KEYBOARDEXAMPLE_APP_UI_TEXT_TEST_TEXT_TEST_VIEW
#define _KEYBOARDEXAMPLE_APP_UI_TEXT_TEST_TEXT_TEST_VIEW


#include <ds/ui/sprite/sprite.h>
#include <ds/app/event_client.h>
#include <ds/ui/sprite/text.h>

namespace example {

class Globals;

/**
* \class example::TextTest
*			Test out multiline wrapping, position getting and stuff
*/
class TextTest final : public ds::ui::Sprite  {
public:
	TextTest(Globals& g);

private:
	void								onAppEvent(const ds::Event&);

	virtual void						onSizeChanged();
	void								layout();

	typedef ds::ui::Sprite				inherited;
	Globals&							mGlobals;

	ds::EventClient						mEventClient;

	ds::ui::Text*						mMessage;
	std::vector<ds::ui::Sprite*>		mCharacterOverlays;

};

} // namespace example

#endif
