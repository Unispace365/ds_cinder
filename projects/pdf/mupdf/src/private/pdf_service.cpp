#include "stdafx.h"

#include "private/pdf_service.h"

#include <ds/app/engine/engine.h>
#include <ds/util/file_meta_data.h>
#include <ds/ui/sprite/pdf.h>

namespace ds {
namespace pdf {

Service::Service(ds::Engine& engine)
	: mEngine(engine)
{
	mEngine.registerSpriteImporter("pdf", [this](ds::ui::SpriteEngine& engine)->ds::ui::Sprite*{
		return new ds::ui::Pdf(mEngine);
	});

	mEngine.registerSpritePropertySetter("pdf_src", [this](ds::ui::Sprite& theSprite, const std::string& theValue, const std::string& fileReferrer){
		std::string absPath = ds::filePathRelativeTo(fileReferrer, theValue);
		ds::ui::Pdf* pdfy = dynamic_cast<ds::ui::Pdf*>(&theSprite);
		if(!pdfy){
			DS_LOG_WARNING("Tried to set the property pdf_src on a non-Pdf sprite");
			return;
		}

		pdfy->setResourceFilename(absPath);
	});
}

Service::~Service(){
	mThread.waitForNoInput();
}

void Service::start(){
	mThread.start(false);
}

} // namespace pdf
} // namespace ds