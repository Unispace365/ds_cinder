#pragma once
#ifndef DS_UI_SPRITE_PDF_H_
#define DS_UI_SPRITE_PDF_H_

#include <cinder/Surface.h>
#include <ds/ui/sprite/sprite.h>
#include "ds/data/resource.h"
#include "pdf_link.h"
namespace ds {
namespace pdf {
class PdfRes;
class Service;
}

namespace ui {

class IPdf : public ds::ui::Sprite {
public:
	IPdf(ds::ui::SpriteEngine& e) : ds::ui::Sprite(e) {}
	// PDF API
	/** Displays the page given, from 1 to getPageCount() */
	virtual void				setPageNum(const int pageNum) = 0;
	/** Returns the page currently being displayed, 1-indexed (page 1 is the first page, there is no page 0) */
	virtual int					getPageNum() const = 0;
	
	/** Returns the number of pages in this document. PDF's are 1-indexed, so the number returned from this function can be set as the last page */
	virtual int					getPageCount() const = 0;

	/** Advance the current page by 1, does not wrap */
	virtual void				goToNextPage() = 0;
	/** Decrement the current page by 1, does not wrap */
	virtual void				goToPreviousPage() = 0;

	/** Called when the page has been requested to change (so you can update UI or whatever) */
	virtual void				setPageChangeCallback(std::function<void()> func) = 0;

	/** When the page renders, will show any links as enabled sprites and send clicked callbacks if they are tapped */
	virtual void				showLinks() {};

	/** Hides any links that were previously shown from showLinks() */
	virtual void				hideLinks() {};

	/** When an internal link has been clicked. Requires links to be shown */
	virtual void				setLinkClickedCallback(std::function<void(ds::pdf::PdfLinkInfo info)> func) { mLinkClickedCallback = func; };
	std::function<void(ds::pdf::PdfLinkInfo)> mLinkClickedCallback;
};

/**
 * \class Pdf
 */
class Pdf : public IPdf {
public:
	static Pdf&					makePdf(SpriteEngine&, Sprite* parent = nullptr);
	// Utility to get a render of the first page of a PDF.
	static ci::Surface8uRef		renderPage(const std::string& path);

	Pdf(ds::ui::SpriteEngine&);

	Pdf&						setResourceFilename(const std::string& filename);
	std::string					getResourceFilename() { return mResourceFilename; }
	Pdf&						setResourceId(const ds::Resource::Id&);

	virtual void				setResource(const ds::Resource& resource) override;

	/// Callback when the page size changes. Highly recommend you listen for this. The sprite will change size before this is called
	void						setPageSizeChangedFn(const std::function<void(void)>&);

	virtual void				onUpdateClient(const UpdateParams&) override;
	virtual void				onUpdateServer(const UpdateParams&) override;

	// IPDF API
	/** Displays the page given, from 1 to getPageCount() */
	virtual void				setPageNum(const int pageNum) override;
	/** Returns the page currently being displayed, 1-indexed (page 1 is the first page, there is no page 0) */
	virtual int					getPageNum() const override;
	/** Returns the number of pages in this document. PDF's are 1-indexed, so the number returned from this function can be set as the last page */
	virtual int					getPageCount() const override;

	/** Advance the current page by 1, does not wrap */
	virtual void				goToNextPage() override;
	/** Decrement the current page by 1, does not wrap */
	virtual void				goToPreviousPage() override;

	/** Called when the page has been requested to change (so you can update UI or whatever) */
	virtual void				setPageChangeCallback(std::function<void()> func) override { mPageChangeCallback = func; }

	/** When the page renders, will show any links as enabled sprites and send clicked callbacks if they are tapped */
	void						showLinks();

	/** Hides any links that were previously shown from showLinks() */
	void						hideLinks();

	/** When an internal link has been clicked. Requires links to be shown */
	void						setLinkClickedCallback(std::function<void(ds::pdf::PdfLinkInfo info)> func) { mLinkClickedCallback = func; };
	// END OF IPDF API

	/** Called when a page has finished drawing. This may also be called if the PDF is re-scaled and redraws again. Expect this to be called many times */
	void						setPageLoadedCallback(std::function<void()> func){ mPageLoadedCallback = func; }

	/** Calls back if the pdf could not be loaded for whatever reason */
	void						setLoadErrorCallback(std::function<void(const std::string& errorMsg)> func) { mErrorCallback = func; };

	/** Returns the current texture for the current page (might not exist so use with caution **/
	ci::gl::TextureRef			getTextureRef() { return mTexture; }

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
		std::vector<pdf::PdfLinkInfo> getLinks();

	private:
		ds::pdf::Service&		mService;
		ds::pdf::PdfRes*		mRes;
	};
	ResHolder					mHolder;

	ci::gl::TextureRef			mTexture;

	// For clients to detect scale changes and re-render
	ci::vec3					mPrevScale;

	void						createLinks();
	void						destroyLinks();
	std::vector<pdf::PdfLink*>	mLinks;
	bool						mShowingLinks;

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
