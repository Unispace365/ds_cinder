#include "private/pdf_res.h"

#include <ds/debug/logger.h>

extern "C" {
#include "MuPDF/fitz.h"
#include "MuPDF/mupdf.h"
}

namespace ds {
namespace pdf {

namespace {

/* LOAD
 * Bundle up the details of loading a PDF
 ******************************************************************/
class Load {
public:
	class Op {
	public:
		Op()			{ }
		virtual ~Op()	{ }
		virtual bool	run(fz_context&, fz_document&, fz_page&) = 0;
	};

public:
	Load() : mCtx(nullptr), mDoc(nullptr), mPage(nullptr) { } //, mXref(NULL), mPage(NULL), mPageObj(NULL)	{ }
	~Load()												{ clear(); }

	bool run(Op& op, const std::string& file, const int pageNumber) {
		clear();
		if (!setTo(file, pageNumber)) return false;
		return op.run(*mCtx, *mDoc, *mPage);
	}

private:
	bool setTo(const std::string& file, const int pageNumber) {
		clear();
		try {
			bool			ans = false;
			if ((mCtx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED)) == nullptr) return false;
			// This is pretty ugly because MuPDF uses custom C++-like error handing that
			// has stringent rules, like you're not allowed to return.
			fz_try(mCtx) {
				if ((mDoc = fz_open_document(mCtx, (char *)file.c_str()))) {
					const int		pageCount = fz_count_pages(mDoc);
					if (!(pageCount <= 0 || pageNumber > pageCount)) {
						if ((mPage = fz_load_page(mDoc, pageNumber - 1))) {
							ans = true;
						}
					}
				}
			}
			fz_always(mCtx)
			{
			}
			fz_catch(mCtx)
			{
			}
			return ans;
		} catch(std::exception& ex) {
			#ifdef _DEBUG
				std::cout << "ofxPdfViewDs.cpp::Load::setTo() exception=" << ex.what() << std::endl;
			#endif
		}
		DS_LOG_WARNING("ds::ui::sprite::Pdf unable to load document \"" << file << "\".");
		return false;
	}

	void clear() {
		if (mPage) fz_free_page(mDoc, mPage);
		if (mDoc) fz_close_document(mDoc);
		if (mCtx) fz_free_context(mCtx);
		mPage = nullptr;
		mDoc = nullptr;
		mCtx = nullptr;
	}

	fz_context*			mCtx;
	fz_document*		mDoc;
	fz_page*			mPage;
};

/* EXAMINE
 * Get the top level details of a PDF
 ******************************************************************/
class Examine : public Load::Op {
public:
	int				mWidth, mHeight, mPageCount;

public:
	Examine() : mWidth(0), mHeight(0), mPageCount(0) { }

	virtual bool	run(fz_context&, fz_document& doc, fz_page& page) {
		fz_rect bounds;
		fz_bound_page(&doc, &page, &bounds);
		if (fz_is_empty_rect(&bounds) || fz_is_infinite_rect(&bounds)) return false;
		mWidth = bounds.x1 - bounds.x0;
		mHeight = bounds.y1 - bounds.y0;
//		mWidth = page.mediabox.x1;
//		mHeight = page.mediabox.y1;
		mPageCount = fz_count_pages(&doc);
		return mWidth > 0 && mHeight > 0 && mPageCount > 0;
	}
};

/* DRAW
 * Draw the page to a surface.
 ******************************************************************/
class Draw : public Load::Op {
public:
	Draw(ds::pdf::PdfRes::Pixels& pixels, const int scaledWidth, const int scaledHeight, const float width, const float height)
		: mPixels(pixels)
		, mScaledWidth(scaledWidth)
		, mScaledHeight(scaledHeight)
		, mWidth(width)
		, mHeight(height)
		{ }

	virtual bool	run(fz_context& ctx, fz_document& doc, fz_page& page) {
		bool					ans = false;
		try {
			fz_pixmap*			pixmap = nullptr;
			fz_device*			device = nullptr;
			// This is pretty ugly because MuPDF uses custom C++-like error handing that
			// has stringent rules, like you're not allowed to return.
			fz_try((&ctx)) {
//				const float		zoom = 1.0f;
				const float		zoom = static_cast<float>(mScaledWidth) / mWidth;
				const float		rotation = 1.0f;
				fz_matrix		transform = fz_identity;
				fz_scale(&transform, zoom, zoom);
	//			transform = fz_concat(transform, fz_rotate(rotation));

				// Take the page bounds and transform them by the same matrix that
				// we will use to render the page.
				fz_rect			rect;
				fz_bound_page(&doc, &page, &rect);
				fz_transform_rect(&rect, &transform);
	//			fz_bbox bbox = fz_round_rect(rect);

				// Create a blank pixmap to hold the result of rendering. The
				// pixmap bounds used here are the same as the transformed page
				// bounds, so it will contain the entire page. The page coordinate
				// space has the origin at the top left corner and the x axis
				// extends to the right and the y axis extends down.
	//			fz_pixmap *pix = fz_new_pixmap_with_bbox(&ctx, fz_device_rgb, bbox);
				int w = mScaledWidth, h = mScaledHeight;
				if (mPixels.setSize(w, h)) {
					mPixels.clearPixels();
					pixmap = fz_new_pixmap_with_data(&ctx, fz_device_rgb, w, h, mPixels.getData());
					if (pixmap) {
						fz_clear_pixmap_with_value(&ctx, pixmap, 0xff);
						// Run the page with the transform.
						device = fz_new_draw_device(&ctx, pixmap);
						if (device) {
							fz_run_page(&doc, &page, device, &transform, NULL);
							ans = true;
						}
					}
				}
			}
			fz_always((&ctx))
			{
			}
			fz_catch((&ctx))
			{
			}
			if (device) fz_free_device(device);
			if (pixmap) fz_drop_pixmap(&ctx, pixmap);
		} catch (std::exception const&) { }
		return ans;
	}

private:
	ds::pdf::PdfRes::Pixels&	mPixels;
	const int					mScaledWidth,
								mScaledHeight;
	const float					mWidth,
								mHeight;
};

} // namespace

/**
 * \class ds::ui::sprite::PdfRes
 */
PdfRes::PdfRes(ds::GlThread& t)
	: ds::GlThreadClient<PdfRes>(t)
	, mPageCount(0)
	, mPixelsChanged(false)
{
	mDrawState.mPageNum = 0;
}

void PdfRes::scheduleDestructor()
{
	performOnWorkerThread(&PdfRes::_destructor);
}

void PdfRes::_destructor()
{
	delete this;
}

PdfRes::~PdfRes()
{
}

bool PdfRes::loadPDF(const std::string& fileName) {
	// I'd really like to do this initial examine stuff in the
	// worker thread, but I suspect the client is expecting this
	// info to be valid as soon as this is called.
	Examine							examine;
	Load							load;
	if (load.run(examine, fileName, 1)) {
		Poco::Mutex::ScopedLock		l(mMutex);
		mFileName = fileName;
		mState.mWidth = examine.mWidth;
		mState.mHeight = examine.mHeight;
		mPageCount = examine.mPageCount;
		return mPageCount > 0;
	}
	return false;
}

float PdfRes::getTextureWidth() const
{
	if (!mTexture) return 0.0f;
	return mTexture.getWidth();
}

float PdfRes::getTextureHeight() const
{
	if (!mTexture) return 0.0f;
	return mTexture.getHeight();
}

#if 0
void Pdf::setAnchorPercent(float xPct, float yPct) {
	Poco::Mutex::ScopedLock		l(mMutex);
	mTex[0].setAnchorPercent(xPct, yPct);
	mTex[1].setAnchorPercent(xPct, yPct);
}

void Pdf::setAnchorPoint(float x, float y) {
	Poco::Mutex::ScopedLock		l(mMutex);
	mTex[0].setAnchorPoint(x, y);
	mTex[1].setAnchorPoint(x, y);
}

void Pdf::resetAnchor() {
	Poco::Mutex::ScopedLock		l(mMutex);
	mTex[0].resetAnchor();
	mTex[1].resetAnchor();
}
#endif

void PdfRes::draw(float x, float y)
{
	if (mPageCount > 0 && mTexture) {
		ci::gl::draw(mTexture, ci::Vec2f(x, y));
	}
}

void PdfRes::goToNextPage()
{
	setPageNum(mState.mPageNum+1);
}

void PdfRes::goToPreviousPage()
{
	setPageNum(mState.mPageNum-1);
}

void PdfRes::setPageNum(int thePageNum)
{
	Poco::Mutex::ScopedLock		l(mMutex);

	if (thePageNum < 1) thePageNum = 1;
	if (thePageNum > mPageCount) thePageNum = mPageCount;
	if (thePageNum == mState.mPageNum) return;
	mState.mPageNum = thePageNum;
}

int PdfRes::getPageNum() {
	Poco::Mutex::ScopedLock		l(mMutex);
	return mState.mPageNum;
}

int PdfRes::getPageCount() {
	Poco::Mutex::ScopedLock		l(mMutex);
	return mPageCount;
}

void PdfRes::setScale(const float theScale)
{
	if (mState.mScale == theScale) return;

	Poco::Mutex::ScopedLock		l(mMutex);
	mState.mScale = theScale;	
}

void PdfRes::update()
{
	// Update the page, if necessary.  Batch process -- once my value has been
	// set, I only need the next pending redraw to perform, everything else
	// is unnecessary.
	if (needsUpdate()) performOnWorkerThread(&PdfRes::_redrawPage, true);

	{
		Poco::Mutex::ScopedLock		l(mMutex);
		if (mPixelsChanged) {
			mPixelsChanged = false;
			if (mPixels.empty()) {
				mTexture = ci::gl::Texture();
			} else {
				if (!mTexture || mTexture.getWidth() != mPixels.getWidth() || mTexture.getHeight() != mPixels.getHeight()) {
					mTexture = ci::gl::Texture(mPixels.getWidth(), mPixels.getHeight());
					if (!mTexture) return;
				}

				GLsizei width = mTexture.getWidth(),
						height = mTexture.getHeight();
				std::vector<GLubyte> emptyData(width * height * 4, 0);

				mTexture.enableAndBind();
				// Cinder Texture doesn't seem to support accessing the data type. I checked the code
				// and it seems to always use GL_UNSIGNED_BYTE, so hopefully that's safe.
				glTexSubImage2D(mTexture.getTarget(), 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, &emptyData[0]);
				glTexSubImage2D(mTexture.getTarget(), 0, 0, 0, mPixels.getWidth(), mTexture.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, mPixels.getData());
				mTexture.unbind();
				mTexture.disable();
				glFinish();
			}
		}
	}


#if 0
	{
		Poco::Mutex::ScopedLock		l(mMutex);
		if (mPixelsChanged) {
			mPixelsChanged = false;
			if (mPixels.empty()) {
				mTexture = ci::gl::Texture();
			} else {
				if (!mTexture || mTexture.getWidth() != mPixels.getWidth() || mTexture.getHeight() != mPixels.getHeight()) {
					mTexture = ci::gl::Texture(mPixels.getWidth(), mPixels.getHeight());
					if (!mTexture) return;
				}
				glBindTexture(mTexture.getTarget(), mTexture.getId());
//				ci::gl::clear(ci::ColorA(0.0f, 0.0f, 0.0f, 0.0f));
				// Cinder Texture doesn't seem to support accessing the data type. I checked the code
				// and it seems to always use GL_UNSIGNED_BYTE, so hopefully that's safe.
//				glTexSubImage2D(mTexture.getTarget(), 0, 0, 0, mPixels.getWidth(), mTexture.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, mPixels.getData());
				glBindTexture(mTexture.getTarget(), 0);
				glFinish();
			}
		}
	}
#endif
}

bool PdfRes::needsUpdate()
{
	Poco::Mutex::ScopedLock		l(mMutex);
	if (mPageCount < 1) return false;
	return mState != mDrawState;
}

void PdfRes::_redrawPage()
{
	// Pop out the pieces we need
	state							drawState;
	std::string						fn;
	{
		Poco::Mutex::ScopedLock		l(mMutex);
		// No reason to regenerate the same page.
		if (mDrawState == mState && mDrawFileName == mFileName) {
			return;
		}
		drawState = mState;
		fn = mFileName;
		// Prevent the main thread from loading the pixels while
		// I'll be modifying them.
		mPixelsChanged = false;
	}

	// Setup parameters
	int scaledWidth = (float)drawState.mWidth * drawState.mScale;
	if (scaledWidth < 1) scaledWidth = 1;
	int scaledHeight = scaledWidth * drawState.mHeight / drawState.mWidth;
	if (scaledHeight < 1) scaledHeight = 1;

	// Render to the texture
	Draw							draw(mPixels, scaledWidth, scaledHeight, drawState.mWidth, drawState.mHeight);
	Load							load;
	if (!load.run(draw, fn, drawState.mPageNum)) {
		DS_LOG_WARNING("ds::pdf::PdfRes unable to rasterize document \"" << fn << "\".");
		return;
	}

	Poco::Mutex::ScopedLock			l(mMutex);
	mPixelsChanged = true;
	mDrawState = drawState;
	// No reason to copy the string which will generally be the same
	if (mDrawFileName != fn) mDrawFileName = fn;
}

/**
 * \class ds::ui::sprite::Pdf::state
 */
PdfRes::state::state()
	: mWidth(0)
	, mHeight(0)
	, mPageNum(1)
	, mScale(1.0f)
{
}

bool PdfRes::state::operator==(const PdfRes::state& o)
{
	return mPageNum == o.mPageNum && mScale == o.mScale && mWidth == o.mWidth && mHeight == o.mHeight;
}

bool PdfRes::state::operator!=(const PdfRes::state& o)
{
	return !(*this == o);
}

/**
 * \class ds::ui::sprite::Pdf::Pixels
 */
PdfRes::Pixels::Pixels()
	: mW(0)
	, mH(0)
	, mData(nullptr)
{
}

PdfRes::Pixels::~Pixels()
{
	delete mData;
}

bool PdfRes::Pixels::empty() const
{
	return mW < 1 || mH < 1 || !mData;
}

bool PdfRes::Pixels::setSize(const int w, const int h)
{
	if (mW == w && mH == h) return true;
	delete mData;
	mData = nullptr;
	mW = 0;
	mH = 0;
	if (w > 0 && h > 0) {
		const int		size = (w * h) * 4;
		mData = new unsigned char[size];
	}
	if (!mData) return false;
	mW = w;
	mH = h;
	return true;
}

unsigned char* PdfRes::Pixels::getData() {
	return mData;
}

void PdfRes::Pixels::clearPixels() {
	if (mW < 1 || mH < 1) return;
	const int			size = mW * mH * 4;
	memset(mData, 0, size);
}

} // using namespace pdf
} // using namespace ds
