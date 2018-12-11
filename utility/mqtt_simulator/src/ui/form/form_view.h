#pragma once
#ifndef _MQTT_SIMULATOR_APP_UI_FORM_VIEW_H_
#define _MQTT_SIMULATOR_APP_UI_FORM_VIEW_H_

#include <ds/ui/layout/smart_layout.h>

namespace downstream {

/**
* \class downstream::FormView
*			Shows the forms to enter stuff to send to mqtt
*/
class FormView : public ds::ui::SmartLayout {
public:
	FormView(ds::ui::SpriteEngine& eng);

	void	sendMessage();

};

} // namespace downstream

#endif

#pragma once
