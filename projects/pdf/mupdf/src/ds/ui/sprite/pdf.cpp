#include "stdafx.h"

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
const DirtyState&	PDF_CURPAGE_DIRTY	= INTERNAL_B_DIRTY;
const char			PDF_FN_ATT			= 80;
const char			PDF_CURPAGE_ATT		= 81;
}

/**
 * Pdf static
 */
void Pdf::installAsServer(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromClient(r);});
}

void Pdf::installAsClient(ds::BlobRegistry& registry) {
	BLOB_TYPE = registry.add([](BlobReader& r) {Sprite::handleBlobFromServer<Pdf>(r);});
}

/**
 * Pdf
 */
Pdf& Pdf::makePdf(SpriteEngine& e, Sprite* parent) {
	return makeAlloc<ds::ui::Pdf>([&e]()->ds::ui::Pdf*{ return new ds::ui::Pdf(e); }, parent);
}

ci::Surface8uRef Pdf::renderPage(const std::string& path) {
	return ds::pdf::PdfRes::renderPage(path);
}

Pdf::Pdf(ds::ui::SpriteEngine& e)
	: ds::ui::IPdf(e)
		, mPageSizeChangeFn(nullptr)
		, mPageSizeCache(0, 0)
		, mHolder(e) 
		, mPrevScale(0.0f, 0.0f, 0.0f)
	, mTexture(nullptr)
{
	// Should be unnecessary, but make sure we reference the static.
	INIT.doNothing();
	mLayoutFixedAspect = true;

	enable(false);
	enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);

	// set some callbacks in case we are ever enabled
	setTapCallback([this](ds::ui::Sprite* sprite, const ci::vec3& pos){
		int count = getPageCount();
		int zeroIndexNextWrapped = (getPageNum() % count);
		setPageNum(zeroIndexNextWrapped + 1);
	});

	setSwipeCallback([this](ds::ui::Sprite* sprite, const ci::vec3& delta){
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

Pdf& Pdf::setResourceFilename(const std::string& filename) {
	mResourceFilename = filename;
	mPageSizeCache = ci::ivec2(0, 0);
	if(!mHolder.setResourceFilename(filename) && mErrorCallback){
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

void Pdf::onUpdateClient(const UpdateParams& p) {
	if(mPrevScale != mScale){
		mHolder.setScale(mScale);
		mPrevScale = mScale;
	}
	mHolder.update();
}

void Pdf::onUpdateServer(const UpdateParams& p) {
	if(mHolder.update()){

		auto theSurface = mHolder.getSurface();
		if(theSurface) {
			if(!mTexture || mTexture->getWidth() != theSurface->getWidth() || mTexture->getHeight() != theSurface->getHeight()) {
				mTexture = ci::gl::Texture2d::create(*theSurface.get());
			} else if(mTexture){
				mTexture->update(*theSurface.get());
			}

			mHolder.clearSurface();
		} else {
			mTexture = nullptr;
		}

		const ci::ivec2			page_size(mHolder.getPageSize());
		if(mPageSizeCache != page_size) {
			mPageSizeCache = page_size;
			if(mPageSizeCache.x < 1 || mPageSizeCache.y < 1){
				DS_LOG_WARNING("Received no size from muPDF!");
			}
			setSize(static_cast<float>(mPageSizeCache.x), static_cast<float>(mPageSizeCache.y));


			if(mPageSizeChangeFn) mPageSizeChangeFn();
		}

		if(mPageLoadedCallback){
			mPageLoadedCallback();
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
	s << "PDF (" << mResourceFilename << ")" << std::endl;
	ds::ui::Sprite::writeState(s, tab);
	s << std::endl;
}
#endif

void Pdf::onScaleChanged() {
	ds::ui::Sprite::onScaleChanged();
	mHolder.setScale(mScale);
}

void Pdf::drawLocalClient() {
	if(!mTexture) {
		return;
	}

	auto tw = mTexture->getWidth();
	auto th = mTexture->getHeight();
	if(tw < 1 || th < 1){
		return;
	}

	mTexture->bind();

	if(mRenderBatch){
		mRenderBatch->draw();
	} else if(mCornerRadius > 0.0f){
		ci::gl::drawSolidRoundedRect(ci::Rectf(0.0f, 0.0f, getWidth(), getHeight()), mCornerRadius, 0, ci::vec2(0, 0), ci::vec2(1, 1));
	} else {
		ci::gl::drawSolidRect(ci::Rectf(0.0f, 0.0f, getWidth(), getHeight()), ci::vec2(0, 0), ci::vec2(1, 1));
	}

	mTexture->unbind();
}

void Pdf::writeAttributesTo(ds::DataBuffer &buf) {
	ds::ui::Sprite::writeAttributesTo(buf);

	if (mDirty.has(PDF_FN_DIRTY)) {
		buf.add(PDF_FN_ATT);
		buf.add(mResourceFilename);
	}

	if(mDirty.has(PDF_CURPAGE_DIRTY)){
		buf.add(PDF_CURPAGE_ATT);
		buf.add(mHolder.getPageNum());
	}
}

void Pdf::readAttributeFrom(const char attributeId, ds::DataBuffer &buf) {
	if (attributeId == PDF_FN_ATT) {
		setResourceFilename(buf.read<std::string>());
	} else if(attributeId == PDF_CURPAGE_ATT) {
		const int			curPage = buf.read<int>();
		setPageNum(curPage);
	} else {
		ds::ui::Sprite::readAttributeFrom(attributeId, buf);
	}
}

/**
 * \class Pdf
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

bool Pdf::ResHolder::setResourceFilename(const std::string& filename) {
	clear();
	bool success = false;
	mRes = new ds::pdf::PdfRes(mService.mThread);
	if (mRes) {
		success = mRes->loadPDF(ds::Environment::expand(filename));
	}

	if(!success){
		DS_LOG_WARNING("Couldn't load " << filename << " in pdf res holder");
	}

	return success;
}

bool Pdf::ResHolder::update() {
	if (mRes) {
		return mRes->update();
	}

	return false;
}


ci::Surface8uRef Pdf::ResHolder::getSurface() {
	if(mRes) return mRes->getSurface();
	return nullptr;
}


void Pdf::ResHolder::clearSurface() {
	if(mRes) {
		mRes->clearSurface();
	}
}

void Pdf::ResHolder::setScale(const ci::vec3& scale) {
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

ci::ivec2 Pdf::ResHolder::getPageSize() const {
	if (!mRes) return ci::ivec2(0, 0);
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
