#pragma once

#include <ds/ui/layout/smart_layout.h>

namespace waffles {

class TemplateBase;

/**
* \class waffles::TransitionLayer
*			The background layer. Holds either media or the ambient particle background
*/
class TransitionLayer : public ds::ui::SmartLayout  {
public:
	TransitionLayer(ds::ui::SpriteEngine& eng);

private:
	void startTransition();

};

} // namespace waffles

