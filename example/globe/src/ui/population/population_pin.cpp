#include "population_pin.h"

#include "ds/ui/interface_xml/interface_xml_importer.h"
#include "event/app_events.h"
#include <app/globals.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>

namespace globe_example {
PopulationPin::PopulationPin(Globals& g)
  : GlobePin(g.mEngine)
  , mGlobals(g) {

	setCenter(0.5, 0.0);

	std::map<std::string, ds::ui::Sprite*> spriteMap;
	ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/pin_content.xml"), spriteMap);
	mPrimaryLayout = dynamic_cast<ds::ui::LayoutSprite*>(spriteMap["root_layout"]);
	mMessage	   = dynamic_cast<ds::ui::Text*>(spriteMap["message"]);
	mVisitor	   = dynamic_cast<ds::ui::Text*>(spriteMap["visitor"]);
	mQrButton	   = dynamic_cast<ds::ui::Image*>(spriteMap["qr_icon"]);

	mPrimaryLayout->enable(true);
	mPrimaryLayout->setTapCallback([this](ds::ui::Sprite*, const ci::vec3) {
		std::cout << "Pin Tap!" << std::endl;
		mGlobals.mEngine.getNotifier().notify(GlobeFocusEvent(getLat(), getLong()));
	});
}

void PopulationPin::layout() {
	if (mPrimaryLayout) {
		mPrimaryLayout->runLayout();
		mPrimaryLayout->tweenAnimateOn(true);
	}
}

void PopulationPin::setData(ds::model::LocationRef ref) {
	setLongLat(ref.getLong(), ref.getLat());
	setLayer(ref.getLayer());
	mModel = ref;
	layout();

	if (mVisitor && mMessage) {
		mVisitor->setText(mModel.getName());
		mMessage->setText(mModel.getPopulation());
	}
}


} // namespace globe_example