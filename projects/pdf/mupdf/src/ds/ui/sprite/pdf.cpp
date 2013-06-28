#include "ds/ui/sprite/pdf.h"

#include <ds/app/app.h>
#include <ds/debug/logger.h>
#include <ds/ui/sprite/sprite_engine.h>
#include "private/pdf_res.h"
#include "private/pdf_service.h"

namespace ds {
namespace ui {

namespace {
// Statically initialize the world class. Done here because the Body is
// guaranteed to be referenced by the final application.
class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			ds::pdf::Service*		w = new ds::pdf::Service();
			if (!w) throw std::runtime_error("Can't create ds::pdf::Service");
			e.addService("pdf", *w);
		});

	}
	void			doNothing() { }
};
Init				INIT;
}

/**
 * \class ds::ui::sprite::Pdf
 */
Pdf::Pdf(ds::ui::SpriteEngine& e)
	: inherited(e)
	, mHolder(e)
{
	// Should be unnecessary, but make sure we reference the static.
	INIT.doNothing();

	setTransparent(false);
	setUseShaderTextuer(true);
}

Pdf& Pdf::setResourceFilename(const std::string& filename)
{
	mHolder.setResourceFilename(filename);
	return *this;
}

void Pdf::updateClient(const UpdateParams& p)
{
	inherited::updateClient(p);
	mHolder.update();
}

void Pdf::updateServer(const UpdateParams& p)
{
	inherited::updateServer(p);
	mHolder.update();
}

void Pdf::drawLocalClient()
{
	inherited::drawLocalClient();
	mHolder.drawLocalClient();
}

/**
 * \class ds::ui::sprite::Pdf
 */
Pdf::ResHolder::ResHolder(ds::ui::SpriteEngine& e)
	: mService(static_cast<ds::pdf::Service&>(e.getService("pdf")))
	, mRes(nullptr)
{
}

Pdf::ResHolder::~ResHolder()
{
	clear();
}

void Pdf::ResHolder::clear()
{
	if (mRes) {
		mRes->scheduleDestructor();
		mRes = nullptr;
	}
}

void Pdf::ResHolder::setResourceFilename(const std::string& filename)
{
	clear();
	mRes = new ds::pdf::PdfRes(mService.mThread);
	if (mRes) {
		mRes->loadPDF(filename);
	}
}

void Pdf::ResHolder::update()
{
	if (mRes) {
		mRes->update();
	}
}

void Pdf::ResHolder::drawLocalClient()
{
	if (mRes) {
		mRes->draw(0.0f, 0.0f);
	}
}

} // using namespace ui
} // using namespace ds
