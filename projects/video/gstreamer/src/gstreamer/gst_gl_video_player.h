#pragma once
#include "cinder\Cinder.h"
#include "cinder\app\App.h"
#include "cinder\gl\Texture.h"
#include "cinder\Timer.h"
#include "cinder\Thread.h"
#include <gst/gst.h>
#include <gst/gstbin.h>
#include <gst/video/video.h>
#include <gst/app/gstappsink.h>
#include <sstream>

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace ci::gl;

#ifndef GST_MIN_SEEK_WAIT
	#define GST_MIN_SEEK_WAIT 0.03333333333333333333
#endif

enum GstPixelFormat {
	GST_PIXELS_RGB,
	GST_PIXELS_RGBA,
	GST_PIXELS_BGRA
};

/*

*/
enum VideoPlaybackLoopMode {
	LOOP_NONE,
	LOOP_NORMAL,
	LOOP_PINGPONG
};

/* 
	hack 

	Note: this "hack" comes straight from the GStreamer gl-plugins example test source code.
*/
typedef struct _GstGLBuffer GstGLBuffer;
struct _GstGLBuffer
{
  GstBuffer buffer;

  GObject *obj;

  gint width;
  gint height;
  GLuint texture;
};

class GstGLVideoPlayer {

public:
	GstGLVideoPlayer();
	~GstGLVideoPlayer();

	void init(string uri, GstPixelFormat format = GST_PIXELS_RGB);

	// Do NOT use the load method; only use the init method.
	// See the load method's definition for the reason.
	void load(string uri, GstPixelFormat format = GST_PIXELS_RGB);

	void play();
	void pause();
	void draw();
	void update();

	int	getWidth(){return mWidth; }
	int getHeight(){return mHeight; }

	gint64 getPositionInNanos();
	float getPosition();
	void setRate(float rate);
	void setPosition(float pos);
	void setLoopMode(VideoPlaybackLoopMode mode);
	bool getIsDone();

	gl::Texture * getTexture() const;

protected:

private:

	string mFilename;
	bool mIsPaused;
	bool mIsLoaded;
	bool mEOSOccurred;
	int mFormat; // Pixel buffer format
	int mBpp; // Bits per pixel
	float mRate;
	gint64 mDurationNanos; // Duration of the video in nanoseconds
	VideoPlaybackLoopMode mLoopMode;
	Timer mSeekStopwatch;

	boost::mutex lock;

	GstElement	* mPlaybin; // playbin2
	string		  mBinName; // Name of our GstBin.
	// GstElement	* mBin; // Bin with input sink, pipes to glupload and fakesink for messaging.
	// GstElement	* mGlupload; // glupload plugin
	GstElement	* mFakesink; // fakesink for message handling
	GstElement  * mAppsink; // appsink for buffer pulling and stream messages
	GstElement  * mCapsFilter;
	GstPad		* mCapsFilterSinkPad; // Sink pad for attaching our ghostpad
	GstPad		* mGhostpad; // Ghostpad on our bin which links to the mPlaybin

	// These Asynchronous Queues _were_ used when we were dealing with fakesink
	// but they're not longer used for now.
	// The basic logic was this:
	//		1) New buffer comes in asynchronously.
	//		2) We shove the buffer into an "input" queue.
	//		3) Our player's update() method is called during the main application's update() routine.
	//		4) We pull the buffer out of the input queue, process it, then stick it in the corresponding output queue
	//		5) 
	GAsyncQueue * mQueueInputBuffer;
	GAsyncQueue * mQueueOutputBuffer;
	GAsyncQueue * mQueuePrerollInputBuffer;
	GAsyncQueue * mQueuePrerollOutputBuffer;

	shared_ptr<gl::Texture> mTex;
	GLuint mTexture;
	gint mTextureWidth;
	gint mTextureHeight;

	int	mWidth;
	int mHeight;

	// Methods that our signals call when we've got changes to the texture information from GStreamer
	void onGstGLTextureUpdate();
	void onGstGLPrerollTextureUpdate();
	void onGstGLTextureUpdateFromBuffer(GstGLBuffer * buffer);
	void onGstEndOfStream();

	// Signals for handling updates from GStreamer
	friend void gstOnFakesinkHandoff (GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data);
	friend void gstOnFakesinkPrerollHandoff (GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data);
	friend void gstOnNewBusMessage (GstBus * bus, GstMessage * msg, gpointer user_data);
	friend GstPad * gstOnVideoPad(GstElement * playbin, gint stream, gpointer user_data);
	
	// Callback handlers for GStreamer's appsink plugin
	friend GstFlowReturn	gstOnNewBufferFromSource (GstAppSink * elt, void * data);
	friend GstFlowReturn	gstOnNewPrerollFromSource (GstAppSink * elt, void * data);
	friend void				gstOnEOSFromSource (GstAppSink * elt, void * data);
	
};
