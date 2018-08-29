#include "stdafx.h"

#include "form_view.h"

#include <ds/ui/sprite/sprite_engine.h>

#include <ds/ui/soft_keyboard/entry_field.h>
#include <ds/ui/button/sprite_button.h>

#include "events/app_events.h"


namespace downstream {

FormView::FormView(ds::ui::SpriteEngine& eng)
	: ds::ui::SmartLayout(eng, "form_view.xml")
{
	listenToEvents<MqttMessageEvent>([this](auto& e) {
		setSpriteText("mqtt_message", e.mMessage);
	});

	auto sendButton = getSprite<ds::ui::SpriteButton>("send_message.the_button");
	if(sendButton) {
		sendButton->setClickFn([this] {
			sendMessage();
		});
	}

	auto entryField = getSprite<ds::ui::EntryField>("entry_field");
	if(entryField) {
		entryField->setNativeKeyboardCallback([this](ci::app::KeyEvent& event)->bool {
			if(event.getCode() == ci::app::KeyEvent::KEY_RETURN) {
				sendMessage();
				return true;
			}
			return false;
		});
		mEngine.registerEntryField(entryField);
	}
}




void FormView::sendMessage() {
	auto entryField = getSprite<ds::ui::EntryField>("entry_field");
	if(entryField) {
		mEngine.getNotifier().notify(SendMqttMessageEvent(entryField->getCurrentTextString()));
	}
}

} // namespace downstream

