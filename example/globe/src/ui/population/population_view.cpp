#include "population_view.h"

#include <app/globals.h>
#include <ds/app/environment.h>
#include <ds/app/event_notifier.h>
#include <ds/ui/sprite/sprite_engine.h>

#include "event/app_events.h"

#include "ds/ui/interface_xml/interface_xml_importer.h"

#include "ui/globe/globe_view.h"
#include "ui/population/population_pin.h"

namespace globe_example {

PopulationView::PopulationView(Globals& g)
  : ds::ui::Sprite(g.mEngine)
  , mGlobals(g)
  , mEventClient(g.mEngine.getNotifier(),
				 [this](const ds::Event* m) {
					 if (m) this->onAppEvent(*m);
				 })
  , mPrimaryLayout(nullptr)
  , mUSButton(nullptr)
  , mAfricaButton(nullptr)
  , mEuroButton(nullptr)
  , mGlobe(nullptr) {

	mGlobe = new ds::ui::GlobeView(mGlobals.mEngine);
	if (mGlobe) {
		addChildPtr(mGlobe);
		for (auto loc : mGlobals.mLocations) {
			PopulationPin* gp = nullptr;
			gp				  = new PopulationPin(mGlobals);
			if (gp) {
				gp->setData(loc);
				mGlobe->addPin(gp);
			}
		}
		mGlobe->clearFilters();
		for (int i = 1; i < 4; ++i) {
			mGlobe->addFilter(i);
		}
		mGlobe->setFocusOffset(-5.0, 12.0);
	}


	std::map<std::string, ds::ui::Sprite*> spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/globe_filters.xml"), spriteMap);
	mPrimaryLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["root_layout"]);

	mUSButton	  = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["us_button.the_button"]);
	mEuroButton	  = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["europe_button.the_button"]);
	mAfricaButton = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["africa_button.the_button"]);


	if (mUSButton) {
		mUSButton->setTapCallback([this](ds::ui::Sprite* s, const ci::vec3 v) {
			mGlobe->clearFilters();
			mGlobe->addFilter(1);
		});
	}

	if (mEuroButton) {
		mEuroButton->setTapCallback([this](ds::ui::Sprite* s, const ci::vec3 v) {
			mGlobe->clearFilters();
			mGlobe->addFilter(2);
		});
	}

	if (mAfricaButton) {
		mAfricaButton->setTapCallback([this](ds::ui::Sprite* s, const ci::vec3 v) {
			mGlobe->clearFilters();
			mGlobe->addFilter(3);
		});
	}

	if (mPrimaryLayout) {
		mPrimaryLayout->runLayout();
		mPrimaryLayout->tweenAnimateOn();
		mPrimaryLayout->sendToFront();
	}
}

void PopulationView::onAppEvent(const ds::Event& in_e) {
	if (in_e.mWhat == GlobeFocusEvent::WHAT()) {
		auto _e = dynamic_cast<const GlobeFocusEvent&>(in_e);
		if (mGlobe) {
			mGlobe->rotateTo(_e.mLatitude, _e.mLongitude);
		}
	}
}

} // namespace globe_example