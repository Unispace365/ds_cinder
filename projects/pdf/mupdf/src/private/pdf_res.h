#pragma once
#ifndef PRIVATE_PDFRES_H_
#define PRIVATE_PDFRES_H_

#include <Poco/Mutex.h>
#include <Poco/Timestamp.h>
#include <cinder/Surface.h>
#include <cinder/gl/Texture.h>
#include <ds/thread/gl_thread.h>

namespace ds {
namespace pdf {

/**
 * \class ds::ui::sprite::PdfRes
 */
class PdfRes : public ds::GlThreadClient<PdfRes> {
public:
	PdfRes(ds::GlThread&);
	// Clients should never delete this class, instead schedule it for deletion and consider it invalid.
	void scheduleDestructor();

private:
	virtual ~PdfRes();

public:
	bool loadPDF(const std::string &theFileName);

	float					getTextureWidth() const;
	float					getTextureHeight() const;

	void update();
#if 0
	void setAnchorPercent(float xPct, float yPct);	//set the anchor as a percentage of the image width/height ( 0.0-1.0 range )
	void setAnchorPoint(float x, float y);					//set the anchor point in pixels
	void resetAnchor();															//resets the anchor to (0, 0)
#endif

	void draw(float x, float y);

	float getWidth() const { return (float)mState.mWidth; }
	float getHeight() const { return (float)mState.mHeight; }
	void setPageNum(int thePageNum);
	int getPageCount();
	void goToNextPage();
	void goToPreviousPage();
	void setScale(const float theScale);

protected:
	// worker thread calls
	void _destructor();
	void _redrawPage();

private:
	struct state {
		state();
		bool		operator==(const state&);
		bool		operator!=(const state&);

		int			mWidth, mHeight, mPageNum;
		float		mScale;
	};

public:
	// Store my pixel data, color space is RGBA, 8 bits per pixel.
	class Pixels {
	public:
		Pixels();
		~Pixels();

		bool				empty() const;
		bool				setSize(const int w, const int h);
		int					getWidth() const		{ return mW; }
		int					getHeight() const		{ return mH; }
		unsigned char*		getData();

	private:
		int					mW, mH;
		unsigned char*		mData;
	};

private:
	bool						needsUpdate();

	Poco::Mutex					mMutex;

	// MAIN THREAD
	ci::gl::Texture				mTexture;

	// WORKER THREAD

	// SHARED
	int							mPageCount;		// Page count < 1 means no PDF has been loaded
	Pixels						mPixels;		// The buffer of data.
	bool						mPixelsChanged;	// Indicates the main thread needs to load in the pixels.
	std::string					mFileName;
	state						mState;			// Store the current state as set by the main thread.  This data is only
													// read by the worker thread, so it's safe to read it in the main without a lock.
	state						mDrawState;		// Store the state used to generate the current active texture
	std::string					mDrawFileName;	// Store so I can avoid redrawing duplicate pages
};

} // namespace pdf
} // namespace ds

#endif // PRIVATE_PDFRES_H_