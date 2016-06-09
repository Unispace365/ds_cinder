#include "ds/ui/sprite/pdf.h"

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/blob_reader.h>
#include <ds/app/environment.h>
#include <ds/data/data_buffer.h>
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
			ds::pdf::Service*		w = new ds::pdf::Service(e);
			if(w){
				e.addService("pdf", *w);
			} else {
				DS_LOG_WARNING("Can't create ds::pdf::Service");
			}

			e.installSprite([](ds::BlobRegistry& r){ds::ui::Pdf::installAsServer(r);},
							[](ds::BlobRegistry& r){ds::ui::Pdf::installAsClient(r);});
		});

	}
	void			doNothing() { }
};
Init				INIT;

char				BLOB_TYPE			= 0;
const DirtyState&	PDF_FN_DIRTY		= INTERNAL_A_DIRTY;
const DirtyState&	PDF_PAGEMODE_DIRTY	= INTERNAL_B_DIRTY;
const DirtyState&	PDF_CURPAGE_DIRTY	= INTERNAL_C_DIRTY;
const char			PDF_FN_ATT			= 80;
const char			PDF_PAGEMODE_ATT	= 81;
const char			PDF_CURPAGE_ATT		= 82;
}

/**
 * \class ds::ui::sprite::Pdf static
 */
void Pdf::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Pdf::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Pdf>(r);});
}

/**
 * \class ds::ui::sprite::Pdf
 */
Pdf& Pdf::makePdf(SpriteEngine& e, Sprite* parent) {
	return makeAlloc<ds::ui::Pdf>([&e]()->ds::ui::Pdf*{ return new ds::ui::Pdf(e); }, parent);
}

ci::Surface8u Pdf::renderPage(const std::string& path) {
	return ds::pdf::PdfRes::renderPage(path);
}

Pdf::Pdf(ds::ui::SpriteEngine& e)
		: inherited(e)
		, mPageSizeMode(kConstantSize)
		, mPageSizeChangeFn(nullptr)
		, mPageSizeCache(0, 0)
		, mHolder(e) 
		, mPrevScale(0.0f, 0.0f, 0.0f)
{
	// Should be unnecessary, but make sure we reference the static.
	INIT.doNothing();
	mLayoutFixedAspect = true;

	enable(false);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);

	// set some callbacks in case we are ever enabled
	this->setTapCallback([this](ds::ui::Sprite* sprite, const ci::Vec3f& pos){
		int count = getPageCount();
		int zeroIndexNextWrapped = (getPageNum() % count);
		setPageNum(zeroIndexNextWrapped + 1);
	});

	this->setSwipeCallback([this](ds::ui::Sprite* sprite, const ci::Vec3f& delta){
		int diff = 0;

		if(delta.x < -20.0f){
			diff = 1;
		} else if(delta.x > 20.0f){
			diff = -1;
		}

		if(diff != 0){
			int count = getPageCount();
			int zeroIndexNextWrapped = ((getPageNum() - 1 + diff + count) % count);
			setPageNum(zeroIndexNextWrapped + 1);
		}
	});

	mBlobType = BLOB_TYPE;
	setTransparent(false);
	setUseShaderTexture(true);
}

Pdf& Pdf::setPageSizeMode(const PageSizeMode& m) {
	mPageSizeMode = m;
	mHolder.setPageSizeMode(m);
	markAsDirty(PDF_PAGEMODE_DIRTY);
	return *this;
}

Pdf& Pdf::setResourceFilename(const std::string& filename) {
	mResourceFilename = filename;
	mPageSizeCache = ci::Vec2i(0, 0);
	if(!mHolder.setResourceFilename(filename, mPageSizeMode) && mErrorCallback){
		std::stringstream errorStream;
		errorStream << "PDF could not be loaded at " << filename;
		std::string errorStr = errorStream.str();
		mErrorCallback(errorStr);
	}
	mHolder.setScale(mScale);
	setSize(mHolder.getWidth(), mHolder.getHeight());
	markAsDirty(PDF_FN_DIRTY);
	return *this;
}

Pdf &Pdf::setResourceId(const ds::Resource::Id &resourceId) {
	try {
		ds::Resource            res;
		if (mEngine.getResources().get(resourceId, res)) {
			Sprite::setSizeAll(res.getWidth(), res.getHeight(), mDepth);
			std::string filename = res.getAbsoluteFilePath();
			setResourceFilename(filename);
		}
	}
	catch (std::exception const& ex) {
		DS_LOG_WARNING("ERROR Pdf::setResourceFilename() ex=" << ex.what());
		return *this;
	}
	return *this;
}

void Pdf::setPageSizeChangedFn(const std::function<void(void)>& fn) {
	mPageSizeChangeFn = fn;
}

void Pdf::updateClient(const UpdateParams& p) {
	inherited::updateClient(p);
	if(mPrevScale != mScale){
		mHolder.setScale(mScale);
		mPrevScale = mScale;
	}
	mHolder.update();
}

void Pdf::updateServer(const UpdateParams& p) {
	inherited::updateServer(p);
	if(mHolder.update() && mPageLoadedCallback){
		mPageLoadedCallback();
	}
	if (mPageSizeMode == kAutoResize) {
		const ci::Vec2i			page_size(mHolder.getPageSize());
		if (mPageSizeCache != page_size) {
			mPageSizeCache = page_size;
			setSize(static_cast<float>(mPageSizeCache.x), static_cast<float>(mPageSizeCache.y));
			if (mPageSizeChangeFn) mPageSizeChangeFn();
		}
	}
}

void Pdf::setPageNum(const int pageNum) {
	mHolder.setPageNum(pageNum);
	markAsDirty(PDF_CURPAGE_DIRTY);
	if(mPageChangeCallback) mPageChangeCallback();
}

int Pdf::getPageNum() const {
	return mHolder.getPageNum();
}

int Pdf::getPageCount() const {
	return mHolder.getPageCount();
}

void Pdf::goToNextPage() {
	mHolder.goToNextPage();
	markAsDirty(PDF_CURPAGE_DIRTY);
	if(mPageChangeCallback) mPageChangeCallback();
}

void Pdf::goToPreviousPage() {
	mHolder.goToPreviousPage();
	markAsDirty(PDF_CURPAGE_DIRTY);
	if(mPageChangeCallback) mPageChangeCallback();
}

#ifdef _DEBUG
void Pdf::writeState(std::ostream &s, const size_t tab) const {
	for (size_t k=0; k<tab; ++k) s << "\t";
	s << "PDF (" << mResourceFilename << ", mode=" << mPageSizeMode << ")" << std::endl;
	inherited::writeState(s, tab);
	s << std::endl;
}
#endif

void Pdf::onScaleChanged() {
	inherited::onScaleChanged();
	mHolder.setScale(mScale);
}

void Pdf::drawLocalClient() {
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

void Pdf::writeAttributesTo(ds::DataBuffer &buf) {
	inherited::writeAttributesTo(buf);

	if (mDirty.has(PDF_FN_DIRTY)) {
		buf.add(PDF_FN_ATT);
		buf.add(mResourceFilename);
	}
	if (mDirty.has(PDF_PAGEMODE_DIRTY)) {
		buf.add(PDF_PAGEMODE_ATT);
		buf.add<int32_t>(static_cast<int32_t>(mPageSizeMode));
	}

	if(mDirty.has(PDF_CURPAGE_DIRTY)){
		buf.add(PDF_CURPAGE_ATT);
		buf.add(mHolder.getPageNum());
	}
}

void Pdf::readAttributeFrom(const char attributeId, ds::DataBuffer &buf) {
	if (attributeId == PDF_FN_ATT) {
		setResourceFilename(buf.read<std::string>());
	} else if (attributeId == PDF_PAGEMODE_ATT) {
		const int32_t		mode = buf.read<int32_t>();
		if (mode == 0) setPageSizeMode(kConstantSize);
		else if (mode == 1) setPageSizeMode(kAutoResize);
	} else if(attributeId == PDF_CURPAGE_ATT) {
		const int			curPage = buf.read<int>();
		setPageNum(curPage);
	} else {
		inherited::readAttributeFrom(attributeId, buf);
	}
}

/**
 * \class ds::ui::sprite::Pdf
 */
Pdf::ResHolder::ResHolder(ds::ui::SpriteEngine& e)
	: mService(e.getService<ds::pdf::Service>("pdf"))
	, mRes(nullptr)
{
}

Pdf::ResHolder::~ResHolder() {
	clear();
}

void Pdf::ResHolder::clear() {
	if (mRes) {
		mRes->scheduleDestructor();
		mRes = nullptr;
	}
}

bool Pdf::ResHolder::setResourceFilename(const std::string& filename, const PageSizeMode& m) {
	clear();
	bool success = false;
	mRes = new ds::pdf::PdfRes(mService.mThread);
	if (mRes) {
		success = mRes->loadPDF(ds::Environment::expand(filename), m);
	}

	return success;
}

bool Pdf::ResHolder::update() {
	if (mRes) {
		return mRes->update();
	}

	return false;
}

void Pdf::ResHolder::drawLocalClient()
{
	if (mRes) {
		mRes->draw(0.0f, 0.0f);
	}
}

void Pdf::ResHolder::setScale(const ci::Vec3f& scale) {
	if (mRes) {
		mRes->setScale(scale.x);
	}
}

void Pdf::ResHolder::setPageSizeMode(const PageSizeMode& m) {
	if (mRes) {
		mRes->setPageSizeMode(m);
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

void Pdf::ResHolder::setPageNum(const int pageNum) {
	if (mRes) mRes->setPageNum(pageNum);
}

int Pdf::ResHolder::getPageNum() const {
	if (mRes) return mRes->getPageNum();
	return 0;
}

int Pdf::ResHolder::getPageCount() const {
	if (mRes) return mRes->getPageCount();
	return 0;
}

ci::Vec2i Pdf::ResHolder::getPageSize() const {
	if (!mRes) return ci::Vec2i(0, 0);
	return mRes->getPageSize();
}

void Pdf::ResHolder::goToNextPage() {
	if (mRes) mRes->goToNextPage();
}

void Pdf::ResHolder::goToPreviousPage()
{
	if (mRes) mRes->goToPreviousPage();
}

} // using namespace ui
} // using namespace ds
