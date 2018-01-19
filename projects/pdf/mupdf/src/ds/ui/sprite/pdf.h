#pragma once
#ifndef DS_UI_SPRITE_PDF_H_
#define DS_UI_SPRITE_PDF_H_

#include <cinder/Surface.h>
#include <ds/ui/sprite/sprite.h>
#include "ds/data/resource.h"
namespace ds {
namespace pdf {
class PdfRes;
class Service;
}

namespace ui {

/**
 * \class ds::ui::sprite::Pdf
 */
class Pdf : public ds::ui::Sprite {
public:
	static Pdf&					makePdf(SpriteEngine&, Sprite* parent = nullptr);
	// Utility to get a render of the first page of a PDF.
	static ci::Surface8uRef		renderPage(const std::string& path);

	Pdf(ds::ui::SpriteEngine&);

	Pdf&						setResourceFilename(const std::string& filename);
	Pdf&						setResourceId(const ds::Resource::Id&);
	// Callback when the page size changes. Highly recommend you listen for this. The sprite will change size before this is called
	void						setPageSizeChangedFn(const std::function<void(void)>&);

	virtual void				onUpdateClient(const UpdateParams&) override;
	virtual void				onUpdateServer(const UpdateParams&) override;

	// PDF API
	/** Displays the page given, from 1 to getPageCount() */
	void						setPageNum(const int pageNum);
	/** Returns the page currently being displayed, 1-indexed (page 1 is the first page, there is no page 0) */
	int							getPageNum() const;
	/** Returns the number of pages in this document. PDF's are 1-indexed, so the number returned from this function can be set as the last page */
	int							getPageCount() const;

	/** Advance the current page by 1, does not wrap */
	void						goToNextPage();
	/** Decrement the current page by 1, does not wrap */
	void						goToPreviousPage();

	/** Called when a page has finished drawing. This may also be called if the PDF is re-scaled and redraws again. Expect this to be called many times */
	void						setPageLoadedCallback(std::function<void()> func){ mPageLoadedCallback = func; }

	/** Called when the page has been requested to change (so you can update UI or whatever) */
	void						setPageChangeCallback(std::function<void()> func){ mPageChangeCallback = func; }

	/** Calls back if the pdf could not be loaded for whatever reason */
	void						setLoadErrorCallback(std::function<void(const std::string& errorMsg)> func){ mErrorCallback = func; };

#ifdef _DEBUG
	virtual void				writeState(std::ostream&, const size_t tab) const;
#endif

protected:
	virtual void				onScaleChanged();
	virtual void				drawLocalClient();
	
	virtual void				writeAttributesTo(ds::DataBuffer&);
	virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

private:

	// STATE
	std::string					mResourceFilename;
	std::function<void(void)>	mPageSizeChangeFn;
	// CACHE
	ci::ivec2					mPageSizeCache;

	// It'd be nice just have the PdfRes in a unique_ptr,
	// but it has rules around destruction
	class ResHolder {
	public:
		ResHolder(ds::ui::SpriteEngine&);
		~ResHolder();

		void					clear();

		/// Returns true if the PDF was loaded successfully
		bool					setResourceFilename(const std::string& filename);

		/// Returns true if pixels were updated in this update
		bool					update();
		ci::Surface8uRef		getSurface();
		void					clearSurface();
		void					setScale(const ci::vec3&);
		float					getWidth() const;
		float					getHeight() const;
		void					setPageNum(const int pageNum);
		int						getPageNum() const;
		int						getPageCount() const;
		ci::ivec2				getPageSize() const;
		void					goToNextPage();
		void					goToPreviousPage();

	private:
		ds::pdf::Service&		mService;
		ds::pdf::PdfRes*		mRes;
	};
	ResHolder					mHolder;

	ci::gl::TextureRef			mTexture;

	// For clients to detect scale changes and re-render
	ci::vec3					mPrevScale;

	std::function<void()>		mPageLoadedCallback;
	std::function<void()>		mPageChangeCallback;
	std::function<void(const std::string&)> mErrorCallback;

	// Initialization
public:
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);
};

} // namespace ui
} // namespace ds

#endif // DS_UI_SPRITE_PDF_H_