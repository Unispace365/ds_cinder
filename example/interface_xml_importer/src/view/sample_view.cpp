#include "sample_view.h"

#include <ds/app/engine/engine_cfg.h>
#include <ds/app/environment.h>
#include <ds/debug/debug_defines.h>
#include <ds/ui/interface_xml/interface_xml_importer.h>
#include <ds/ui/sprite/image.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/tween/tweenline.h>
#include <ds/physics/body_builder.h>
#include <ds/physics/collision.h>
#include <ds/math/math_defs.h>

#include "app/globals.h"

#pragma warning(disable: 4355)

namespace importer_example {

SampleView::SampleView(Globals& g)
	: inherited(g.mEngine)
	, mGlobals(g)
	, mTitle(nullptr)
	, mBody(nullptr)
	, mSampleImage(nullptr)
{


	ds::ui::XmlImporter::loadXMLto(this, mGlobals.mXmlImporterMap["sample.xml"], mSpriteMap);

	mTitle = dynamic_cast<ds::ui::Text*>(mSpriteMap["title"]);
	mBody = dynamic_cast<ds::ui::MultilineText*>(mSpriteMap["body"]);
	mSampleImage = dynamic_cast<ds::ui::Image*>(mSpriteMap["sample_image"]);

	// always check xml-loaded sprites, in case the xml didn't load correct or that sprite was removed or whatever
	if(mSampleImage){
		mSampleImage->enable(true);
		mSampleImage->enableMultiTouch(ds::ui::MULTITOUCH_NO_CONSTRAINTS);
	}

	

}
} // namespace importer_example
