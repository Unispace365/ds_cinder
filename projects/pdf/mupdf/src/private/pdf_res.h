#ifndef PRIVATE_PDFRES_H_
#define PRIVATE_PDFRES_H_

#include <mutex>

#include <cinder/Surface.h>
#include <cinder/gl/Texture.h>

#include <ds/thread/gl_thread.h>
#include <ds/ui/sprite/pdf.h>
#include <ds/ui/sprite/pdf_link.h>

namespace ds::pdf {

/**
 * \class ds::ui::sprite::PdfRes
 */
class PdfRes : public ds::GlThreadClient<PdfRes> {
  public:
	// Utility to get a render of the first page of a PDF.
	static ci::Surface8uRef renderPage(const std::string& path);

	PdfRes(ds::GlThread&);
	// Clients should never delete this class, instead schedule it for deletion and consider it invalid.
	void scheduleDestructor();

  private:
	virtual ~PdfRes();


  public:
	// Store my pixel data, color space is RGBA, 8 bits per pixel.
	struct Pixels {
		Pixels();
		~Pixels();

		bool		   empty() const;
		bool		   setSize(const int w, const int h);
		void		   clearPixels();
		void		   deleteData();
		unsigned char* mData;
		int			   mDataSize;
		int			   mW, mH;
	};


	bool loadPDF(const std::string& theFileName);

	/// Returns true if the pixels were updated on this pass
	bool update();

	ci::Surface8uRef getSurface() { return mSurface; }
	void			 clearSurface();

	std::vector<PdfLinkInfo> getLinks();

	float	  getWidth() const;
	float	  getHeight() const;
	void	  setPageNum(int thePageNum);
	int		  getPageNum() const;
	int		  getPageCount() const;
	ci::ivec2 getPageSize() const;
	void	  goToNextPage();
	void	  goToPreviousPage();
	void	  setScale(const float theScale);

  protected:
	// worker thread calls
	void _destructor();
	void _redrawPage();

  private:
	struct state {
		state();
		bool operator==(const state&);
		bool operator!=(const state&);

		int	  mWidth, mHeight, mPageNum;
		float mScale;
		// NOTE: These items are not part of the equality test
		ci::ivec2				 mPageSize;
		std::vector<PdfLinkInfo> mLinks;
	};

	bool needsUpdate();

	mutable std::mutex mMutex;

	// MAIN THREAD
	ci::Surface8uRef mSurface;
	Pixels			 mSurfacePixels;

	// WORKER THREAD

	// SHARED
	bool		mRequestUpdate;
	int			mPageCount;		// Page count < 1 means no PDF has been loaded
	Pixels		mPixels;		// The buffer of data.
	bool		mPixelsChanged; // Indicates the main thread needs to load in the pixels.
	std::string mFileName;
	state		mState;		   // Store the current state as set by the main thread.  This data is only
							   // read by the worker thread, so it's safe to read it in the main without a lock.
	state		mDrawState;	   // Store the state used to generate the current active texture
	std::string mDrawFileName; // Store so I can avoid redrawing duplicate pages

	bool mPrintedError; // to prevent a ton of warnings flooding the output
};

} // namespace ds::pdf

#endif // PRIVATE_PDFRES_H_
