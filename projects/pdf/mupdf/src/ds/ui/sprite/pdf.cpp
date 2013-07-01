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
	mHolder.setScale(mScale);
	setSize(mHolder.getWidth(), mHolder.getHeight());
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

void Pdf::onScaleChanged()
{
	inherited::onScaleChanged();
	mHolder.setScale(mScale);
}

void Pdf::drawLocalClient()
{
	inherited::drawLocalClient();

	// When drawing, we have to go through some histrionics because we
	// want this sprite to look the same as other sprites to the outside
	// world, but internally, we have a buffer that is rendered at the
	// scaled size, not the sprite size.
	const float				tw = mHolder.getTextureWidth(),
							th = mHolder.getTextureHeight();
	if (tw < 1.0f || th < 1.0f) return;

	const float				targetw = getWidth()*mScale.x,
							targeth = getHeight()*mScale.y;
    ci::gl::pushModelView();

	// To draw properly, we first have to turn off whatever scaling has
	// been applied, then apply a new scale to compensate for any mismatch
	// between my current texture size and my display size.
	const ci::Vec3f			turnOffScale(1.0f/mScale.x, 1.0f/mScale.y, 1.0f);
	const ci::Vec3f			newScale(targetw/tw, targeth/th, 1.0f);
    ci::gl::multModelView(ci::Matrix44f::createScale(turnOffScale));
    ci::gl::multModelView(ci::Matrix44f::createScale(newScale));

	mHolder.drawLocalClient();

    ci::gl::popModelView();
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

void Pdf::ResHolder::setScale(const ci::Vec3f& scale)
{
	if (mRes) {
		mRes->setScale(scale.x);
	}
}

float Pdf::ResHolder::getWidth() const
{
	if (mRes) return mRes->getWidth();
	return 0.0f;
}

float Pdf::ResHolder::getHeight() const
{
	if (mRes) return mRes->getHeight();
	return 0.0f;
}

float Pdf::ResHolder::getTextureWidth() const
{
	if (mRes) return mRes->getTextureWidth();
	return 0.0f;
}

float Pdf::ResHolder::getTextureHeight() const
{
	if (mRes) return mRes->getTextureHeight();
	return 0.0f;
}

} // using namespace ui
} // using namespace ds
