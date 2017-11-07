#pragma once

#ifndef DS_EXAMPLE_POPULATION_VIEW
#define DS_EXAMPLE_POPULATION_VIEW

#include "ds/ui/sprite/sprite_engine.h"
#include "ds/ui/sprite/sprite.h"

#include "ui/globe/pins/globe_pin.h"

#include <ds/app/event_client.h>

#include <ds/ui/button/sprite_button.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/text.h>
#include <ds/ui/layout/layout_sprite.h>

#include "model/location_model.h"

namespace ds {
	namespace ui {
		class GlobeView;
	}
}

namespace globe_example {

	class Globals;

	class PopulationView :public ds::ui::Sprite
	{
	public:

		PopulationView(Globals& g);
		PopulationView::~PopulationView() {}

	private:

		void onAppEvent(const ds::Event&);

		ds::ui::LayoutSprite*				mPrimaryLayout;

		ds::ui::SpriteButton*				mEuroButton;
		ds::ui::SpriteButton*				mUSButton;
		ds::ui::SpriteButton*				mAfricaButton;

		Globals&							mGlobals;
		ds::EventClient						mEventClient;
		ds::ui::GlobeView*					mGlobe;

	};

}

#endif