#pragma once

#ifndef DS_EXAMPLE_POPULATION_PIN
#define DS_EXAMPLE_POPULATION_PIN

#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/sprite_engine.h"

#include "ui/globe/pins/globe_pin.h"

#include <ds/ui/layout/layout_sprite.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/text.h>

#include "model/location_model.h"

namespace globe_example {
class Globals;

class PopulationPin : public ds::ui::GlobePin {
  public:
	PopulationPin(Globals& g);
	PopulationPin::~PopulationPin() {}

	void setData(ds::model::LocationRef);

  private:
	void layout();

	ds::model::LocationRef mModel;

	ds::ui::LayoutSprite* mPrimaryLayout;
	ds::ui::LayoutSprite* mListLayout;

	Globals& mGlobals;

	ds::ui::Image* mQrButton;
	ds::ui::Text*  mMessage;
	ds::ui::Text*  mVisitor;
};

} // namespace globe_example

#endif