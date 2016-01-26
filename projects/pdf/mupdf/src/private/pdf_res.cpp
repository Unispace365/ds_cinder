#include "private/pdf_res.h"

#include <ds/debug/logger.h>

extern "C" {
#include "mupdf/fitz.h"
#include "mupdf/pdf.h"
#include "mupdf/fitz/pixmap.h"
#include "mupdf/fitz/colorspace.h"
#include "mupdf/fitz/context.h"
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

			fz_register_document_handlers(mCtx);

			// This is pretty ugly because MuPDF uses custom C++-like error handing that
			// has stringent rules, like you're not allowed to return.
			fz_try(mCtx) {
				if ((mDoc = fz_open_document(mCtx, (char *)file.c_str()))) {
					const int		pageCount = fz_count_pages(mCtx, mDoc);
					if (!(pageCount <= 0 || pageNumber > pageCount)) {
						if ((mPage = fz_load_page(mCtx, mDoc, pageNumber - 1))) {
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
		try{ if(mPage) fz_drop_page(mCtx, mPage); } catch(...){}
		try{ if(mDoc) fz_drop_document(mCtx, mDoc); } catch(...){}
		try{ if(mCtx) fz_drop_context(mCtx); } catch(...){}
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

	virtual bool	run(fz_context& ctx, fz_document& doc, fz_page& page) {
		fz_rect bounds;
		fz_bound_page(&ctx, &page, &bounds);
		if (fz_is_empty_rect(&bounds) || fz_is_infinite_rect(&bounds)) return false;
		mWidth = bounds.x1 - bounds.x0;
		mHeight = bounds.y1 - bounds.y0;
//		mWidth = page.mediabox.x1;
//		mHeight = page.mediabox.y1;
		mPageCount = fz_count_pages(&ctx, &doc);
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
		, mScale(1.0f)
		, mWidth(width)
		, mHeight(height)
		, mMode(0)
		{ }

	Draw(ds::pdf::PdfRes::Pixels& pixels, const float scale)
		: mPixels(pixels)
		, mScaledWidth(0)
		, mScaledHeight(0)
		, mScale(scale)
		, mWidth(0)
		, mHeight(0)
		, mMode(1)
		{ }


	const ci::Vec2i&	getPageSize() const {
		return mPageSize;
	}

	virtual bool	run(fz_context& ctx, fz_document& doc, fz_page& page) {
		if(mScaledWidth > 12000.0f || mScaledHeight > 12000.0f){
			return false;
		}
		if (mMode == 0) return runMode0(ctx, doc, page);
		if (mMode == 1) return runMode1(ctx, doc, page);
		return false;
	}

private:
	virtual bool	runMode0(fz_context& ctx, fz_document& doc, fz_page& page) {
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
				fz_bound_page(&ctx, &page, &rect);
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
					pixmap = fz_new_pixmap_with_data(&ctx, fz_device_rgb(&ctx), w, h, mPixels.getData());
					if (pixmap) {
						fz_clear_pixmap_with_value(&ctx, pixmap, 0xff);
						// Run the page with the transform.
						device = fz_new_draw_device(&ctx, pixmap);
						if (device) {
							fz_run_page(&ctx, &page, device, &transform, NULL);
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
			if (device) fz_drop_device(&ctx, device);
			if (pixmap) fz_drop_pixmap(&ctx, pixmap);
		} catch (std::exception const&) { }
		return ans;
	}

	virtual bool	runMode1(fz_context& ctx, fz_document& doc, fz_page& page) {
		bool					ans = false;
		try {
			fz_pixmap*			pixmap = nullptr;
			fz_device*			device = nullptr;
			// This is pretty ugly because MuPDF uses custom C++-like error handing that
			// has stringent rules, like you're not allowed to return.
			fz_try((&ctx)) {
				if (!getPageSize(ctx, page)) return false;

				mWidth = static_cast<float>(mPageSize.x);
				mHeight = static_cast<float>(mPageSize.y);
				mScaledWidth = static_cast<int>(mScale * mWidth);
				mScaledHeight = static_cast<int>(mScale * mHeight);

				const float		zoom = static_cast<float>(mScaledWidth) / mWidth;
				const float		rotation = 1.0f;
				fz_matrix		transform = fz_identity;
				fz_scale(&transform, zoom, zoom);

				// Take the page bounds and transform them by the same matrix that
				// we will use to render the page.
				fz_rect			rect;
				fz_bound_page(&ctx, &page, &rect);
				fz_transform_rect(&rect, &transform);

				// Create a blank pixmap to hold the result of rendering. The
				// pixmap bounds used here are the same as the transformed page
				// bounds, so it will contain the entire page. The page coordinate
				// space has the origin at the top left corner and the x axis
				// extends to the right and the y axis extends down.
				int w = mScaledWidth, h = mScaledHeight;
				if (mPixels.setSize(w, h)) {
					mPixels.clearPixels();
					pixmap = fz_new_pixmap_with_data(&ctx, fz_device_rgb(&ctx), w, h, mPixels.getData());
					if (pixmap) {
						fz_clear_pixmap_with_value(&ctx, pixmap, 0xff);
						// Run the page with the transform.
						device = fz_new_draw_device(&ctx, pixmap);
						if (device) {
							fz_run_page(&ctx, &page, device, &transform, NULL);
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
			if (device) fz_drop_device(&ctx, device);
			if (pixmap) fz_drop_pixmap(&ctx, pixmap);
		} catch (std::exception const&) { }
		return ans;
	}

	virtual bool	getPageSize(fz_context& ctx, fz_page& page) {
		fz_rect bounds;
		fz_bound_page(&ctx, &page, &bounds);
		if (fz_is_empty_rect(&bounds) || fz_is_infinite_rect(&bounds)) return false;
		mPageSize.x = bounds.x1 - bounds.x0;
		mPageSize.y = bounds.y1 - bounds.y0;
		return mPageSize.x > 0 && mPageSize.y > 0;
	}

	ds::pdf::PdfRes::Pixels&	mPixels;
	int							mScaledWidth,
								mScaledHeight;
	const float					mScale;
	float						mWidth,
								mHeight;
	int							mMode;
	ci::Vec2i					mOutSize,
								mPageSize;
};

} // namespace

/**
 * \class ds::ui::sprite::PdfRes
 */
ci::Surface8u PdfRes::renderPage(const std::string& path) {
	ci::Surface8u			s;

	Examine					examine;
	Load					load;
	const int				page_num = 1;
	if (!load.run(examine, path, page_num)) return s;
	if (examine.mWidth < 1 || examine.mHeight < 1) return s;

	Pixels					pixels;
	pixels.setSize(examine.mWidth, examine.mHeight);
	Draw					draw(pixels, examine.mWidth, examine.mHeight, examine.mWidth, examine.mHeight);
	if (!load.run(draw, path, page_num)) return s;

	s = ci::Surface8u(pixels.getData(), examine.mWidth, examine.mHeight, examine.mWidth * 4, ci::SurfaceChannelOrder(ci::SurfaceChannelOrder::BGRA));
	if (s) return s.clone(true);
	return s;
}

PdfRes::PdfRes(ds::GlThread& t)
		: ds::GlThreadClient<PdfRes>(t)
		, mPageCount(0)
		, mPixelsChanged(false) 
		, mPrintedError(false)
{
	mDrawState.mPageNum = 0;
}

void PdfRes::scheduleDestructor() {
	performOnWorkerThread(&PdfRes::_destructor);
}

void PdfRes::_destructor() {
	delete this;
}

PdfRes::~PdfRes() {
}

bool PdfRes::loadPDF(const std::string& fileName, const ds::ui::Pdf::PageSizeMode& pageSizeMode) {
	mPrintedError = false;
	// I'd really like to do this initial examine stuff in the
	// worker thread, but I suspect the client is expecting this
	// info to be valid as soon as this is called.
	Examine							examine;
	Load							load;
	if (load.run(examine, fileName, 1)) {
		std::lock_guard<decltype(mMutex)>	l(mMutex);
		mFileName = fileName;
		mState.mWidth = examine.mWidth;
		mState.mHeight = examine.mHeight;
		mState.mPageSizeMode = pageSizeMode;
		mPageCount = examine.mPageCount;
		return mPageCount > 0;
	}
	return false;
}

float PdfRes::getTextureWidth() const {
	if (!mTexture) return 0.0f;
	return mTexture.getWidth();
}

float PdfRes::getTextureHeight() const {
	if (!mTexture) return 0.0f;
	return mTexture.getHeight();
}

void PdfRes::draw(float x, float y) {
	if (mPageCount > 0 && mTexture) {
		ci::gl::draw(mTexture, ci::Vec2f(x, y));
	}
}

void PdfRes::goToNextPage() {
	setPageNum(mState.mPageNum+1);
}

void PdfRes::goToPreviousPage() {
	setPageNum(mState.mPageNum-1);
}

float PdfRes::getWidth() const {
	if (mTexture) return mTexture.getWidth();
	return (float)mState.mWidth;
}

float PdfRes::getHeight() const {
	if (mTexture) return mTexture.getHeight();
	return (float)mState.mHeight;
}

void PdfRes::setPageNum(int thePageNum) {
	std::lock_guard<decltype(mMutex)>		l(mMutex);

	if (thePageNum < 1) thePageNum = 1;
	if (thePageNum > mPageCount) thePageNum = mPageCount;
	if (thePageNum == mState.mPageNum) return;
	mState.mPageNum = thePageNum;
}

int PdfRes::getPageNum() const {
	std::lock_guard<decltype(mMutex)>		l(mMutex);
	return mState.mPageNum;
}

int PdfRes::getPageCount() const {
	std::lock_guard<decltype(mMutex)>		l(mMutex);
	return mPageCount;
}

ci::Vec2i PdfRes::getPageSize() const {
	std::lock_guard<decltype(mMutex)>		l(mMutex);
	return mState.mPageSize;
}

void PdfRes::setScale(const float theScale) {
	if (mState.mScale == theScale) return;

	std::lock_guard<decltype(mMutex)>		l(mMutex);
	mState.mScale = theScale;	
}

void PdfRes::setPageSizeMode(const ds::ui::Pdf::PageSizeMode& m) {
	std::lock_guard<decltype(mMutex)>		l(mMutex);
	mState.mPageSizeMode = m;	
}

void PdfRes::update() {
	// Update the page, if necessary.  Batch process -- once my value has been
	// set, I only need the next pending redraw to perform, everything else
	// is unnecessary.
	if (needsUpdate()) performOnWorkerThread(&PdfRes::_redrawPage, true);

	{
		std::lock_guard<decltype(mMutex)>		l(mMutex);
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
		mState.mPageSize = mDrawState.mPageSize;
	}
}

bool PdfRes::needsUpdate() {
	std::lock_guard<decltype(mMutex)>		l(mMutex);
	if (mPageCount < 1) return false;
	return mState != mDrawState;
}

void PdfRes::_redrawPage() {
	// Pop out the pieces we need
	bool							printedError;
	state							drawState;
	std::string						fn;
	{
		std::lock_guard<decltype(mMutex)>		l(mMutex);
		
		// No reason to regenerate the same page.
		const float scaleEpsilon = 1e-2f;
		bool isScaleCloseEnough = (abs(mDrawState.mScale - mState.mScale) < scaleEpsilon);

		bool isSameStateIgnoringScale = false;

		float savedDrawStateScale = mDrawState.mScale;
		float savedStateScale = mState.mScale;
		mDrawState.mScale = 1.0f;
		mState.mScale = 1.0f;
		isSameStateIgnoringScale = (mDrawState == mState);
		mDrawState.mScale = savedDrawStateScale;
		mState.mScale = savedStateScale;

		bool isSameFile = (mDrawFileName == mFileName);
		
		if(isScaleCloseEnough && isSameStateIgnoringScale && isSameFile) {
			return;
		}
		drawState = mState;
		fn = mFileName;
		// Prevent the main thread from loading the pixels while
		// I'll be modifying them.
		mPixelsChanged = false;
		printedError = mPrintedError;
	}

	// Setup parameters
	int scaledWidth = (float)drawState.mWidth * drawState.mScale;
	if (scaledWidth < 1) scaledWidth = 1;
	int scaledHeight = scaledWidth * drawState.mHeight / drawState.mWidth;
	if (scaledHeight < 1) scaledHeight = 1;

	// Prevent trying to draw a PDF that's too large (can cause a memory overload and crashy thingy)
	if(scaledWidth > 8000 || scaledHeight > 8000){
		float newW = (float)scaledWidth;
		float newH = (float)scaledHeight;
		float asp = newW / newH;
		newW = 8000;
		newH = 8000;
		if(asp < 1.0f){
			newW = newH * asp;
		} else {
			newH = newW / asp;
		}
		scaledWidth = (int)floorf(newW);
		scaledHeight = (int)floorf(newH);
	}

	// Render to the texture
	if (drawState.mPageSizeMode == ds::ui::Pdf::kConstantSize) {
		Draw							draw(mPixels, scaledWidth, scaledHeight, drawState.mWidth, drawState.mHeight);
		Load							load;
		if (!load.run(draw, fn, drawState.mPageNum)) {
			if(!printedError){
				DS_LOG_WARNING("ds::pdf::PdfRes unable to rasterize document \"" << fn << "\".");
				std::lock_guard<decltype(mMutex)>			l(mMutex);
				mPrintedError = true;
			}
			return;
		}
	} else if (drawState.mPageSizeMode == ds::ui::Pdf::kAutoResize) {
		Draw							draw(mPixels, drawState.mScale);
		Load							load;
		if(!load.run(draw, fn, drawState.mPageNum)) {
			if(!printedError){
				DS_LOG_WARNING("ds::pdf::PdfRes unable to rasterize document \"" << fn << "\".");
				std::lock_guard<decltype(mMutex)>			l(mMutex);
				mPrintedError = true;
			}
			return;
		}
		drawState.mPageSize = draw.getPageSize();
	}

	std::lock_guard<decltype(mMutex)>			l(mMutex);
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
		, mPageSize(0, 0) {
}

bool PdfRes::state::operator==(const PdfRes::state& o) {
	return mPageNum == o.mPageNum && mScale == o.mScale && mWidth == o.mWidth && mHeight == o.mHeight && mPageSizeMode == o.mPageSizeMode;
}

bool PdfRes::state::operator!=(const PdfRes::state& o) {
	return !(*this == o);
}

/**
 * \class ds::ui::sprite::Pdf::Pixels
 */
PdfRes::Pixels::Pixels()
		: mW(0)
		, mH(0)
		, mData(nullptr) {
}

PdfRes::Pixels::~Pixels() {
	delete mData;
}

bool PdfRes::Pixels::empty() const {
	return mW < 1 || mH < 1 || !mData;
}

bool PdfRes::Pixels::setSize(const int w, const int h) {
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
