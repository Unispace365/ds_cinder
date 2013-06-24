#include "gst_gl_video_player.h"

static bool gst_is_thread_support_initialized = false;
static bool gst_is_inited = false;
static int gst_bin_counter = 0;

/*
	Define some no-operation callbacks for setting the callback handlers on the
	appsink plugin.
*/
void eos_noop(GstAppSink* appSink, gpointer data) {}
GstFlowReturn new_preroll_noop(GstAppSink* appSink, gpointer data) { return GST_FLOW_OK; }
GstFlowReturn new_buffer_noop(GstAppSink* appSink, gpointer data) { return GST_FLOW_OK; }
GstFlowReturn new_buffer_list_noop(GstAppSink* appSink, gpointer data) { return GST_FLOW_OK; }

/*
	----------------------------------------------------------------------------
	Define callbacks for the appsink which handle three different events:

	1) New preroll buffer.

	2) New stream buffer.

	3) End-of-stream (EOS) message.
*/
GstFlowReturn gstOnNewPrerollFromSource (GstAppSink * elt, gpointer data)
{
	GstGLVideoPlayer * player = (GstGLVideoPlayer*) data;

	GstGLBuffer *buffer = (GstGLBuffer *) gst_app_sink_pull_preroll(GST_APP_SINK (elt));

	// Put this buffer in our queue to deal with during our update.
	// g_async_queue_push( player->mQueuePrerollInputBuffer, buffer );

	player->onGstGLTextureUpdateFromBuffer(buffer);
	gst_buffer_unref( (GstBuffer*) buffer );

	return GST_FLOW_OK;
}

GstFlowReturn gstOnNewBufferFromSource (GstAppSink * elt, gpointer data)
{
	GstGLVideoPlayer * player = (GstGLVideoPlayer*) data;

	GstGLBuffer *buffer = (GstGLBuffer *) gst_app_sink_pull_buffer(GST_APP_SINK (elt));
	
	// Put this buffer in our queue to deal with during our update.
	// g_async_queue_push( player->mQueueInputBuffer, buffer );

	player->onGstGLTextureUpdateFromBuffer(buffer);
	gst_buffer_unref( (GstBuffer*) buffer );

	return GST_FLOW_OK;
}

void gstOnEOSFromSource (GstAppSink * elt, gpointer data)
{
	GstGLVideoPlayer * player = (GstGLVideoPlayer*) data;
	player->onGstEndOfStream();
}

/*
	End of callbacks for the appsink.
	----------------------------------------------------------------------------
*/

/*
	Callback handler for receiving new stream buffers from the fakesink.

	NOTE: This is not used right now.
*/
void gstOnFakesinkHandoff (GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data)
{
	int maxBuffers = 3;

	// Cast our player's instance
	GstGLVideoPlayer* player = (GstGLVideoPlayer*) user_data;
	GstGLBuffer * glBuffer = (GstGLBuffer *) gst_buffer_ref(buffer);

	g_async_queue_push( player->mQueueInputBuffer, glBuffer );
	if(g_async_queue_length(player->mQueueInputBuffer) > maxBuffers) {
		// Notify our player that we've got an updated texture.
		player->onGstGLTextureUpdate();
	}

	if(g_async_queue_length(player->mQueueOutputBuffer) > maxBuffers) {
		GstBuffer *buf_old = (GstBuffer *) g_async_queue_pop (player->mQueueOutputBuffer);
		gst_buffer_unref (buf_old);
	}
}

/*
	Callback handler for receiving new preroll buffers from the fakesink.

	NOTE: This is not used right now.
*/
void gstOnFakesinkPrerollHandoff (GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer user_data)
{
	int maxBuffers = 3;

	// Cast our player's instance
	GstGLVideoPlayer* player = (GstGLVideoPlayer*) user_data;
	GstGLBuffer * glBuffer = (GstGLBuffer *) gst_buffer_ref(buffer);

	g_async_queue_push( player->mQueuePrerollInputBuffer, glBuffer );
	if(g_async_queue_length(player->mQueuePrerollInputBuffer) > maxBuffers) {
		// Notify our player that we've got an updated texture.
		player->onGstGLPrerollTextureUpdate();
	}

	if(g_async_queue_length(player->mQueuePrerollOutputBuffer) > maxBuffers) {
		GstBuffer *buf_old = (GstBuffer *) g_async_queue_pop (player->mQueuePrerollOutputBuffer);
		gst_buffer_unref (buf_old);
	}
}

/*
	Callback handler for receiving new messages from the playbin2's bus.

	NOTE: This is not used right now.
*/
 void gstOnNewBusMessage(GstBus * bus, GstMessage * msg, gpointer user_data)
{
	// Cast our player's instance
	GstGLVideoPlayer* player = (GstGLVideoPlayer*) user_data;
	
	// App::get()->console() << "GStreamer: New " << GST_MESSAGE_TYPE_NAME(msg) << " message." << endl;
	
	switch (GST_MESSAGE_TYPE (msg)) {

		case GST_MESSAGE_STATE_CHANGED:
			{
				GstState oldstate, newstate, pendstate;
				gst_message_parse_state_changed(msg, &oldstate, &newstate, &pendstate);
				// App::get()->console() << "GStremer: State changed from " << getGstStateName(oldstate) << " to " << getGstStateName(newstate) << " (" << getGstStateName(pendstate) << ")" << endl;
			}
			break;

		case GST_MESSAGE_ERROR:
			{
				GError *err;
				gchar *elementName;
				gchar *debug;
				gst_message_parse_error(msg, &err, &debug);
				elementName = gst_element_get_name(GST_MESSAGE_SRC (msg));

				App::get()->console() << "GStreamer Error: Video playback halted. Module " << elementName << " reported: " << err->message << endl;

				g_error_free(err);
				g_free(debug);
				g_free(elementName);

				gst_element_set_state(GST_ELEMENT(player->mPlaybin), GST_STATE_NULL);
			}
			break;

		case GST_MESSAGE_EOS:
			// Here's where we handle the looping logic.
			App::get()->console() << "GStreamer: end of stream message." << endl;

			switch(player->mLoopMode){

				case LOOP_NORMAL:
					{
						GstFormat format = GST_FORMAT_TIME;
						GstSeekFlags flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_SKIP);
						gint64 pos;
						gst_element_query_position(GST_ELEMENT(player->mPlaybin), &format, &pos);

						if(player->mRate > 0) {
							if(!gst_element_seek(GST_ELEMENT(player->mPlaybin),
												player->mRate,
												format,
												flags,
												GST_SEEK_TYPE_SET, 0,
												GST_SEEK_TYPE_SET, GST_CLOCK_TIME_NONE)) {
								App::get()->console() << "GStreamer Error: unable to seek." << endl;
							}
						} else {
							if(!gst_element_seek(GST_ELEMENT(player->mPlaybin),
												player->mRate,
												format,
												flags,
												GST_SEEK_TYPE_SET, 0,
												GST_SEEK_TYPE_SET, player->mDurationNanos)) {
								App::get()->console() << "GStreamer Error: unable to seek." << endl;
							}
						}
					}
					break;

				case LOOP_PINGPONG:
					{
						GstFormat format = GST_FORMAT_TIME;
						GstSeekFlags flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_SKIP);
						gint64 pos;
						gst_element_query_position(GST_ELEMENT(player->mPlaybin), &format, &pos);
						float loopSpeed;
						if(pos>0)
							loopSpeed = -player->mRate;
						else
							loopSpeed = player->mRate;
						if(!gst_element_seek(GST_ELEMENT(player->mPlaybin),
											loopSpeed,
											GST_FORMAT_UNDEFINED,
											flags,
											GST_SEEK_TYPE_NONE,
											0,
											GST_SEEK_TYPE_NONE,
											0)) {
							App::get()->console() << "GStreamer Error: unable to seek." << endl;
						}
					}
					break;

				default:
				break;
			}

		break;

		default:
			App::get()->console() << "GStreamer: unhandled message." << endl;
		break;
	}
}

GstGLVideoPlayer::GstGLVideoPlayer()
	: mIsPaused(false)
	, mIsLoaded(false)
	, mEOSOccurred(false)
	, mDurationNanos(0)
	, mBpp(0)
	, mTexture(0)
	, mTextureWidth(0)
	, mTextureHeight(0)
	, mTex(NULL)
	, mLoopMode(LOOP_NONE)
	, mRate(1.f)
	, mPlaybin(NULL)
	// , mBin(NULL)
	// , mGlupload(NULL)
	, mFakesink(NULL)
	, mAppsink(NULL)
	, mCapsFilterSinkPad(NULL)
	, mGhostpad(NULL)
	, mQueuePrerollInputBuffer(NULL)
	, mQueuePrerollOutputBuffer(NULL)
	, mQueueInputBuffer(NULL)
	, mQueueOutputBuffer(NULL)
{
	App::get()->console() << "GstGLVideoPlayer: Constructor ------------------" << endl;

	if(!gst_is_thread_support_initialized) {
		if(!g_thread_supported()){
			g_thread_init(NULL);
		}
		gst_is_thread_support_initialized = true;
	}

	if(!gst_is_inited) {
		App::get()->console() << "GstGLVideoPlayer: GStreamer initialized." << endl;
		gst_init(NULL, NULL);
		gst_is_inited = true;
	}
}

GstGLVideoPlayer::~GstGLVideoPlayer()
{
	return;
	App::get()->console() << "GstGLVideoPlayer: Destructor called on " << mFilename << endl;
	if(!mSeekStopwatch.isStopped())
		mSeekStopwatch.stop();
	
	// Get rid of callbacks from appsink
	GstAppSinkCallbacks callbacks = {	&eos_noop,
										&new_preroll_noop,
										&new_buffer_noop,
										&new_buffer_list_noop };
	gst_app_sink_set_callbacks(GST_APP_SINK(mAppsink), &callbacks, NULL, NULL);

	g_object_set (G_OBJECT(mPlaybin), "video-sink", NULL, NULL);
	g_object_set (G_OBJECT(mPlaybin), "audio-sink", NULL, NULL);

	gst_element_set_state (GST_ELEMENT (mPlaybin), GST_STATE_NULL);

	// Unref these elements.
	gst_object_unref(mGhostpad);
	gst_object_unref(mCapsFilterSinkPad);
	// gst_object_unref(mCapsFilter);
	// gst_object_unref(mGlupload);
	gst_object_unref(mAppsink);
	// gst_object_unref(mBin);
	gst_object_unref(mPlaybin);
	
	// Need to empty out the async buffer queues so that there's no pending communication between gst and our player.
	if(mQueueInputBuffer) {
		while (g_async_queue_length (mQueueInputBuffer) > 0) {
			GstBuffer *buf = (GstBuffer *) g_async_queue_pop (mQueueInputBuffer);
			gst_buffer_unref (buf);
		}
		g_async_queue_unref( mQueueInputBuffer );
	}

	if(mQueueOutputBuffer) {
		while (g_async_queue_length (mQueueOutputBuffer) > 0) {
			GstBuffer *buf = (GstBuffer *) g_async_queue_pop (mQueueOutputBuffer);
			gst_buffer_unref (buf);
		}
		g_async_queue_unref( mQueueOutputBuffer );
	}

	if(mQueuePrerollInputBuffer) {
		while (g_async_queue_length (mQueuePrerollInputBuffer) > 0) {
			GstBuffer *buf = (GstBuffer *) g_async_queue_pop (mQueuePrerollInputBuffer);
			gst_buffer_unref (buf);
		}
		g_async_queue_unref( mQueuePrerollInputBuffer );
	}

	if(mQueuePrerollOutputBuffer) {
		while (g_async_queue_length (mQueuePrerollOutputBuffer) > 0) {
			GstBuffer *buf = (GstBuffer *) g_async_queue_pop (mQueuePrerollOutputBuffer);
			gst_buffer_unref (buf);
		}
		g_async_queue_unref( mQueuePrerollOutputBuffer );
	}

	App::get()->console() << "GstGLVideoPlayer: done destroying: " << mFilename << endl;
}

void GstGLVideoPlayer::init(string uri, GstPixelFormat format)
{
	// Find the asset.
	//fs::path moviePath = app::getAssetPath( uri );
	//string moviePathStr = moviePath.string();
	string finalPath = "file:///" + uri;
	mFilename.assign(uri);

	mFormat = format;

	// Store the current OpenGL context and device context information.
	HGLRC mContext = wglGetCurrentContext();
	HDC mDC = wglGetCurrentDC();
	// Change the current context.
	wglMakeCurrent(0, 0);
	
	// Now let's create our pipeline for playback.
	// 
	// 	playbin2
	// 	  + property (GstElement) video-sink
	// 
	// 	  video-sink is to be set to our GstBin mBin
	// 
	// 	  --- mBin -------------------------------------------
	// 	  |                                                  |
	// 	  |  ghostpad    capsfilter   glupload     fakesink  |
	// 	  |      +-----------+-----------+------------+      |
	// 	  |   "sink"        sink       sink          sink    |
	// 	  |                                                  |
	// 	  ----------------------------------------------------
	//

	// Create a new playbin2, which is a very capable playback pipeline for all kinds of media:
	// See: http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-plugins/html/gst-plugins-base-plugins-playbin2.html
	mPlaybin = gst_element_factory_make("playbin2", NULL); // Replace NULL with name of instance if needed.
	// Set the URI so the playbin2 knows what we're reading.
	app::App::get()->console() << "GstGLVideoPlayer: Setting the URI of the file to be: " << finalPath << endl;
	g_object_set(G_OBJECT(mPlaybin), "uri", finalPath.c_str(), NULL);

	// create the fakesink, the "Black hole for data"
	// fakesink plugin here: http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer-plugins/html/gstreamer-plugins-fakesink.html
	mFakesink = gst_element_factory_make("fakesink", NULL);  // Replace NULL with name of factory instance

	// Create an appsink element
	// appsink plugin here: http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-libs/html/gst-plugins-base-libs-appsink.html
	mAppsink = gst_element_factory_make("appsink", NULL);
	
	// Make our glupload.
	GstElement * mGlupload = gst_element_factory_make("glupload", NULL);
	g_object_set( G_OBJECT(mGlupload), "external-opengl-context", mContext, NULL);
	
	// Create our capsfilter for filtering out any unwanted streams.
	mCapsFilter = gst_element_factory_make("capsfilter", NULL);
	
	// Create our capabilities and assign to the capsfilter based on the pixel format.
	string mime;
	switch(mFormat) {
		case GST_PIXELS_RGB:
			mime = "video/x-raw-rgb";
			mBpp = 24;
			break;
		case GST_PIXELS_RGBA:
		case GST_PIXELS_BGRA:
			mime = "video/x-raw-rgb";
			mBpp = 32;
			break;
		default:
			mime = "video/x-raw-rgb";
			mBpp=24;
			break;
	}
	
	GstCaps *caps = NULL;
	// Need to create a capabilities that allows for alpha channel masking.
	switch(mFormat) {
	case GST_PIXELS_RGBA:
		caps = gst_caps_new_simple(		mime.c_str(),
										"bpp",        G_TYPE_INT, mBpp,
										"depth",	  G_TYPE_INT, mBpp,
										"endianness", G_TYPE_INT, G_BIG_ENDIAN,
										"red_mask",   G_TYPE_INT, 0xFF000000,
										"green_mask", G_TYPE_INT, 0x00FF0000,
										"blue_mask",  G_TYPE_INT, 0x0000FF00,
										"alpha_mask", G_TYPE_INT, 0x000000FF,
										NULL
									);
		break;
	case GST_PIXELS_BGRA:
		caps = gst_caps_new_simple(		mime.c_str(),
										"bpp",        G_TYPE_INT, mBpp,
										"depth",      G_TYPE_INT, mBpp,
										"endianness", G_TYPE_INT, G_BIG_ENDIAN,
										"blue_mask",  G_TYPE_INT, 0xFF000000,
										"green_mask", G_TYPE_INT, 0x00FF0000,
										"red_mask",   G_TYPE_INT, 0x0000FF00,
										"alpha_mask", G_TYPE_INT, 0x000000FF,
										NULL
									);
		break;
	default:
		caps = gst_caps_new_simple(mime.c_str(), "bpp", G_TYPE_INT, mBpp, NULL);
		break;
	}
	// Assign the capabilities to our capsfilter.
	g_object_set(G_OBJECT(mCapsFilter), "caps", caps, NULL);
	// Don't need this GstCaps instance any longer.
	gst_caps_unref(caps);

	// Start creating a bin.
	// Create our bin where we'll have an exposed sink for hooking up to the playbin2.
	// We need to create a new name so we don't have collisions when we've got multiple videos playing
	ostringstream buf;
	buf << "gstglplayerbin" << gst_bin_counter;
	gst_bin_counter++;
	if(gst_bin_counter > 0xFFFF) {
		gst_bin_counter = 0;
	}
	mBinName = buf.str();
	GstElement * mBin = gst_bin_new( mBinName.c_str() );

	// Stick our elements into our bin and link them up.
	// gst_bin_add_many( GST_BIN(mBin), capsFilter, mGlupload, mFakesink, NULL );
	// gst_element_link_many( capsFilter, mGlupload, mFakesink, NULL );
	gst_bin_add_many( GST_BIN(mBin), mCapsFilter, mGlupload, mAppsink, NULL );
	gst_element_link_many( mCapsFilter, mGlupload, mAppsink, NULL );
	
	// Now add a ghostpad to the bin. Start by getting a reference to the fakesink's sink pad.
	mCapsFilterSinkPad = gst_element_get_static_pad( GST_ELEMENT(mCapsFilter), "sink");
	mGhostpad = gst_ghost_pad_new("sink", mCapsFilterSinkPad); // This associates a new ghostpad with our glupload's sink pad.
	gst_element_add_pad( GST_ELEMENT(mBin), mGhostpad); // Add our new ghostpad to our bin which will become our "sink".

	// Tell our playbin2 that we're using our custom capsfilter->glupload->fakesink bin for handling video buffers
	g_object_set (G_OBJECT(mPlaybin), "video-sink", mBin, NULL);
	
	// Using MS DirectSound as our sound sink for output.
	// TODO: Make this work with other sound sinks for Mac OS.
	GstElement *audioSink = gst_element_factory_make("directsoundsink", NULL);
	g_object_set (G_OBJECT(mPlaybin), "audio-sink", audioSink, NULL);

	// Force sync.
	// gst_base_sink_set_sync(GST_BASE_SINK(mFakesink), TRUE);
	gst_base_sink_set_sync(GST_BASE_SINK(mAppsink), TRUE);

	// Appsink-specific settings.
	// Max out the buffers so as to not destroy memory
	g_object_set( G_OBJECT( mAppsink ), "max-buffers", 5, NULL );
	// we need to tell it to drop old buffers when max amount of queued buffers is reached.
	gst_app_sink_set_drop (GST_APP_SINK(mAppsink), TRUE);

	// Now add some callbacks so we can handle incoming buffers.
	// set the appsink to not emit signals, we are using callbacks instead
	g_object_set (G_OBJECT (mAppsink), "emit-signals", FALSE, NULL);

	GstAppSinkCallbacks gstCallbacks;
	gstCallbacks.eos = &gstOnEOSFromSource;
	// gstCallbacks.new_preroll = &new_preroll_noop;
	// gstCallbacks.new_buffer = &new_buffer_noop;
	gstCallbacks.new_preroll = &gstOnNewPrerollFromSource;
	gstCallbacks.new_buffer = &gstOnNewBufferFromSource;

	// By using callbacks instead of signals, signals will automatically NOT be emitted.
	// It's got lower overhead and it's less expensive.
	// See documentation: http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-libs/html/gst-plugins-base-libs-appsink.html#gst-app-sink-set-callbacks
	gst_app_sink_set_callbacks(GST_APP_SINK(mAppsink), &gstCallbacks, this, NULL);

	/*

	// Note: This seems to work with the fakesink, but we've gone to using an appsink
	//       plugin instead so that we can get the EOS message correctly. The processing
	//		 of the messages coming from the playbin2's bus led to memory errors every
	//		 time we tried to process them, and I think it's because those messages were
	//		 coming in asynchronously from the stream's thread. So as we were processing
	//		 them, the list of messages in the bus's messages was changing. I think.

	// Now let's add all our watch signals.
	g_object_set (G_OBJECT (mFakesink), "signal-handoffs", TRUE, NULL);
	g_signal_connect (mFakesink, "preroll-handoff", G_CALLBACK (gstOnFakesinkPrerollHandoff), this );
	g_signal_connect (mFakesink, "handoff", G_CALLBACK (gstOnFakesinkHandoff), this );

	// Listen to signals coming from our pipeline's bus.
	GstBus *bus = gst_pipeline_get_bus( GST_PIPELINE(mPlaybin) );
	gst_bus_add_signal_watch(bus);
	g_signal_connect( bus, "message::eos", G_CALLBACK (gstOnNewBusMessage), this );
	g_signal_connect( bus, "message::warning", G_CALLBACK (gstOnNewBusMessage), this );
	g_signal_connect( bus, "message::error", G_CALLBACK (gstOnNewBusMessage), this );
	gst_object_unref(bus);

	*/

	// Once we've set everything, we need to move into a paused state, which will trigger everything.
	GstStateChangeReturn stateChangeStatus = gst_element_set_state(GST_ELEMENT(mPlaybin), GST_STATE_PAUSED);
	if(stateChangeStatus == GST_STATE_CHANGE_FAILURE) {
		App::get()->console() << "GstGLVideoPlayer: GST_STATE_CHANGE_FAILURE: error trying to pause the pipeline." << endl;
		App::get()->quit();
	} else if(stateChangeStatus == GST_STATE_CHANGE_ASYNC) {
		GstState state = GST_STATE_PAUSED;
		if( gst_element_get_state( GST_ELEMENT(mPlaybin), &state, NULL, GST_SECOND * 2) != GST_STATE_CHANGE_SUCCESS ) {
			App::get()->console() << "GstGLVideoPlayer: error trying to pause the pipeline." << endl;
			App::get()->quit();
		}
	}

	// Switch back to our regularly scheduled contexts.
	wglMakeCurrent(mDC, mContext);

	// Note: these buffers async queues are currently not used, but I'm keeping them just so we have reference to HOW to use them.
	// We're creating a stack that we pull information about the buffers from.
	mQueueInputBuffer = g_async_queue_new();
	mQueueOutputBuffer = g_async_queue_new();
	mQueuePrerollInputBuffer = g_async_queue_new();
	mQueuePrerollOutputBuffer = g_async_queue_new();
	
	// We're loaded.
	mIsLoaded = true;
	mIsPaused = true;
	mEOSOccurred = false;

	// Get the duration.
	GstFormat timeFormat = GST_FORMAT_TIME;
	gst_element_query_duration( mPlaybin, &timeFormat, &mDurationNanos );

//	GstPad* gstPad = gst_element_get_static_pad( m_GstVideoSink, "sink" );
	gst_video_get_size( GST_PAD( mCapsFilterSinkPad ), &mWidth, &mHeight );

	App::get()->console() << "GstGLVideoPlayer: Duration of the video is: " << mDurationNanos << endl;

	// Unreference our capsfilter and our audiosink now that we're done with them.
	gst_object_unref( audioSink );
	gst_object_unref( mBin );
	gst_object_unref( mGlupload );
}

/*
	This method should not be used. It was created in order to test something that _should_
	work but doesn't. In GStreamer discussion forums, it's stated that the playbin2 pipeline
	plugin _should_ be able to have its "uri" property changed, and everything should be able
	to be reused. What I've discovered is that this is _not_ true. We're keeping it in here
	so that people know this for now.
*/
void GstGLVideoPlayer::load(string uri, GstPixelFormat format)
{
	// Find the asset.
	fs::path moviePath = app::getAssetPath( uri );
	string moviePathStr = moviePath.string();
	string finalPath = "file:///" + moviePathStr;
	mFilename.assign(moviePathStr);

	mFormat = format;
	
	/*
	// Store the current OpenGL context and device context information.
	HGLRC mContext = wglGetCurrentContext();
	HDC mDC = wglGetCurrentDC();
	// Change the current context.
	wglMakeCurrent(0, 0);
	*/

	// Set the URI so the playbin2 knows what we're reading.
	app::App::get()->console() << "GstGLVideoPlayer: Setting the URI of the file to be: " << finalPath << endl;
	g_object_set(G_OBJECT(mPlaybin), "uri", mFilename.c_str(), NULL);

	// Create our capabilities and assign to the capsfilter based on the pixel format.
	string mime;
	switch(mFormat) {
		case GST_PIXELS_RGB:
			mime = "video/x-raw-rgb";
			mBpp = 24;
			break;
		case GST_PIXELS_RGBA:
		case GST_PIXELS_BGRA:
			mime = "video/x-raw-rgb";
			mBpp = 32;
			break;
		default:
			mime = "video/x-raw-rgb";
			mBpp=24;
			break;
	}
	
	GstCaps *caps = NULL;
	// Need to create a capabilities that allows for alpha channel masking.
	switch(mFormat) {
	case GST_PIXELS_RGBA:
		caps = gst_caps_new_simple(		mime.c_str(),
										"bpp",        G_TYPE_INT, mBpp,
										"depth",	  G_TYPE_INT, mBpp,
										"endianness", G_TYPE_INT, G_BIG_ENDIAN,
										"red_mask",   G_TYPE_INT, 0xFF000000,
										"green_mask", G_TYPE_INT, 0x00FF0000,
										"blue_mask",  G_TYPE_INT, 0x0000FF00,
										"alpha_mask", G_TYPE_INT, 0x000000FF,
										NULL
									);
		break;
	case GST_PIXELS_BGRA:
		caps = gst_caps_new_simple(		mime.c_str(),
										"bpp",        G_TYPE_INT, mBpp,
										"depth",      G_TYPE_INT, mBpp,
										"endianness", G_TYPE_INT, G_BIG_ENDIAN,
										"blue_mask",  G_TYPE_INT, 0xFF000000,
										"green_mask", G_TYPE_INT, 0x00FF0000,
										"red_mask",   G_TYPE_INT, 0x0000FF00,
										"alpha_mask", G_TYPE_INT, 0x000000FF,
										NULL
									);
		break;
	default:
		caps = gst_caps_new_simple(mime.c_str(), "bpp", G_TYPE_INT, mBpp, NULL);
		break;
	}
	// Assign the capabilities to our capsfilter.
	g_object_set(G_OBJECT(mCapsFilter), "caps", caps, NULL);
	// Don't need this GstCaps instance any longer.
	gst_caps_unref(caps);

	// Once we've set everything, we need to move into a paused state, which will trigger everything.
	GstStateChangeReturn stateChangeStatus = gst_element_set_state(GST_ELEMENT(mPlaybin), GST_STATE_PAUSED);
	if(stateChangeStatus == GST_STATE_CHANGE_FAILURE) {
		App::get()->console() << "GstGLVideoPlayer: GST_STATE_CHANGE_FAILURE: error trying to pause the pipeline." << endl;
		App::get()->quit();
	} else if(stateChangeStatus == GST_STATE_CHANGE_ASYNC) {
		GstState state = GST_STATE_PAUSED;
		if( gst_element_get_state( GST_ELEMENT(mPlaybin), &state, NULL, GST_SECOND * 10) != GST_STATE_CHANGE_SUCCESS ) {
			App::get()->console() << "GstGLVideoPlayer: error trying to pause the pipeline." << endl;
			App::get()->quit();
		}
	}

	// Change the current context.
	// wglMakeCurrent(mDC, mContext);

	// We're loaded.
	mIsLoaded = true;
	mIsPaused = true;
	mEOSOccurred = false;

	// Get the duration.
	GstFormat timeFormat = GST_FORMAT_TIME;
	gst_element_query_duration( mPlaybin, &timeFormat, &mDurationNanos );

	App::get()->console() << "GstGLVideoPlayer: Duration of the video is: " << mDurationNanos << endl;
}

void GstGLVideoPlayer::play()
{
	if(mIsPaused) {
		GstStateChangeReturn stateChangeStatus = gst_element_set_state(GST_ELEMENT(mPlaybin), GST_STATE_PLAYING);
		/*
		if(stateChangeStatus == GST_STATE_CHANGE_FAILURE) {
			App::get()->console() << "GstGLVideoPlayer: error trying to play the pipeline." << endl;
		} else if(stateChangeStatus == GST_STATE_CHANGE_ASYNC) {
			GstState state = GST_STATE_PLAYING;
			if( gst_element_get_state( GST_ELEMENT(mPlaybin), &state, NULL, GST_SECOND * 2) != GST_STATE_CHANGE_SUCCESS ) {
				App::get()->console() << "GstGLVideoPlayer: error trying to play the pipeline." << endl;
			} else {
				mIsPaused = false;
				return;
			}
		} else if(stateChangeStatus == GST_STATE_CHANGE_SUCCESS) {
			mIsPaused = false;
			return;
		}
		*/
	}
}

void GstGLVideoPlayer::pause()
{
	if(!mIsPaused) {
		GstStateChangeReturn stateChangeStatus = gst_element_set_state(GST_ELEMENT(mPlaybin), GST_STATE_PAUSED);
		/*
		if(stateChangeStatus == GST_STATE_CHANGE_FAILURE) {
			App::get()->console() << "GstGLVideoPlayer: error trying to pause the pipeline." << endl;
		} else if(stateChangeStatus == GST_STATE_CHANGE_ASYNC) {
			GstState state = GST_STATE_PAUSED;
			if( gst_element_get_state( GST_ELEMENT(mPlaybin), &state, NULL, GST_SECOND * 2) != GST_STATE_CHANGE_SUCCESS ) {
				App::get()->console() << "GstGLVideoPlayer: error trying to pause the pipeline." << endl;
			} else {
				mIsPaused = true;
				return;
			}
		} else if(stateChangeStatus == GST_STATE_CHANGE_SUCCESS) {
			mIsPaused = true;
			return;
		}
		*/
	}
}

gint64 GstGLVideoPlayer::getPositionInNanos()
{
	if(mIsLoaded){
		gint64 pos = 0;
		GstFormat format = GST_FORMAT_TIME;
		if( ! gst_element_query_position( GST_ELEMENT(mPlaybin), &format, &pos )){
			App::get()->console() << "GStreamer Error: Was unable to query the stream's current position." << endl;
			return -1;
		}
		return pos;
		return static_cast<gint64>(static_cast<double>(pos)/static_cast<double>(mDurationNanos));
	} else {
		return -1;
	}
}

float GstGLVideoPlayer::getPosition()
{
	if(mIsLoaded) {
		gint64 posInNanos = getPositionInNanos();
		if(posInNanos > -1) {
			return (float) posInNanos / (float) mDurationNanos;
		} else {
			return -1.f;
		}
		
	} else {
		return -1.f;
	}
}

bool GstGLVideoPlayer::getIsDone()
{
	return mEOSOccurred;
}

void GstGLVideoPlayer::setRate(float rate)
{
	if(rate == 0)
		return;

	if(rate == mRate)
		return;
	
	if(mIsLoaded) {
		mRate = rate;
		// Update the playspeed.
		GstFormat format = GST_FORMAT_TIME;
		GstSeekFlags flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SKIP);
		gint64 pos = getPositionInNanos();
		if(mRate > 0) {
			if(!gst_element_seek(GST_ELEMENT(mPlaybin), mRate, format,
					flags,
					GST_SEEK_TYPE_SET, pos,
					GST_SEEK_TYPE_SET, mDurationNanos))
			{
				App::get()->console() << "GStreamer: unable to seek!" << endl;
			}
		} else {
			if(!gst_element_seek(GST_ELEMENT(mPlaybin), mRate, format,
					flags,
					GST_SEEK_TYPE_SET, 0,
					GST_SEEK_TYPE_SET, pos))
			{
				App::get()->console() << "GStreamer: unable to seek!" << endl;
			}
		}
	}
}

void GstGLVideoPlayer::setLoopMode(VideoPlaybackLoopMode mode)
{
	mLoopMode = mode;
	App::get()->console() << "GstGLVideoPlayer: loop mode set to: " << mLoopMode << endl;
}

void GstGLVideoPlayer::setPosition(float pos)
{
	if(mIsLoaded) {

		// We need to make sure we're not calling this more than 30fps (1 / 30.f). GST_MIN_SEEK_WAIT
		if(mSeekStopwatch.isStopped()) {
			mSeekStopwatch.start();
		} else {
			if(mSeekStopwatch.getSeconds() < GST_MIN_SEEK_WAIT) {
				return;
			} else {
				mSeekStopwatch.stop();
				mSeekStopwatch.start();
			}
		}

		GstSeekFlags flags;
		GstFormat format = GST_FORMAT_TIME;
		gint64 curPos = getPositionInNanos();
		gint64 newPos = (guint64)( (double) pos * (double) mDurationNanos );

		if(newPos == curPos)
			return;

		// TODO: 
		//			If we're paused, then we need to do a trick. Basically, we're going
		//			to tell the playbin2 that we're allowed to skip frames when jumping around
		//			This only occurs when the playspeed is set to something > 2.0 or < -2.0.
		//			By doing this, the playbin2 will scrub a lot faster. Or at least that's the idea.

		// First we would need to check if the state is paused, which we're currently not tracking.

		/*
		// If we're paused, we're going to do a trick that lets us skip frames.
		flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SKIP);

		// Notice here that we're creating a temporary speed that enables the GST_SEEK_FLAG_SKIP to work.
		float tempSpeed = newPos > curPos ? 2.0f : -2.0f;
		if(tempSpeed > 0) {
			if(!gst_element_seek(GST_ELEMENT(mPlaybin), tempSpeed, format,
					flags,
					GST_SEEK_TYPE_SET,
					newPos,
					GST_SEEK_TYPE_SET,
					GST_CLOCK_TIME_NONE)) {
				App::get()->console() << "GStreamer: unable to seek!" << endl;
			}
		} else {
			if(!gst_element_seek(GST_ELEMENT(mPlaybin), tempSpeed, format,
					flags,
					GST_SEEK_TYPE_SET,
					0,
					GST_SEEK_TYPE_SET,
					newPos)) {
				App::get()->console() << "GStreamer: unable to seek!" << endl;
			}
		}
		*/

		// This is the logic for a "normal" seek-to-position request.
		flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_SKIP);
		if(mRate>0){
			if(!gst_element_seek(GST_ELEMENT(mPlaybin), mRate, format,
					flags,
					GST_SEEK_TYPE_SET, newPos,
					GST_SEEK_TYPE_SET, mDurationNanos))
			{
				App::get()->console() << "GStreamer: unable to seek!" << endl;
			}
		}else{
			if(!gst_element_seek(GST_ELEMENT(mPlaybin),mRate, 	format,
					flags,
					GST_SEEK_TYPE_SET, 0,
					GST_SEEK_TYPE_SET, newPos))
			{
				App::get()->console() << "GStreamer: unable to seek!" << endl;
			}
		}
	}
}

void GstGLVideoPlayer::onGstEndOfStream()
{
	// Trying a mutex here, but I have no idea if it actually matters.
	lock.lock();
	App::get()->console() << "GstGLVideoPlayer: end of stream occurred." << endl;
	mEOSOccurred = true; // We set this flag so that we can check for it on the next update.
	lock.unlock();
}

void GstGLVideoPlayer::update()
{

	// Force the bus to signal any messages that are on its message stack.
	// This is a blocking call. While the GStreamer GstBus docs explicitly
	// state that you should never use the gst_bus_poll() method, as it's 
	// pure evil, the reality is that we're never running a glib main loop,
	// so we would never be able to receive these messages via signals.
	/*
		GstBus *bus = gst_pipeline_get_bus( GST_PIPELINE(mPlaybin) );
		if(bus) {
			gst_bus_poll( bus, (GST_MESSAGE_STATE_CHANGED), 0 );
		}
		gst_object_unref(bus);
	*/

	if(mEOSOccurred) {
		switch(mLoopMode){

			case LOOP_NORMAL:
				{
					GstFormat format = GST_FORMAT_TIME;
					GstSeekFlags flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_SKIP);
					if(!gst_element_seek(GST_ELEMENT(mPlaybin),
										mRate,
										format,
										flags,
										GST_SEEK_TYPE_SET, 0,
										GST_SEEK_TYPE_SET, mDurationNanos)) {
						App::get()->console() << "GStreamer Error: unable to seek." << endl;
					}
				}
				mEOSOccurred = false;
				break;

			case LOOP_PINGPONG:
				{
					GstFormat format = GST_FORMAT_TIME;
					GstSeekFlags flags = (GstSeekFlags) (GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_SKIP);
					// Need to reverse the speed of playback
					mRate = -mRate;
					if(!gst_element_seek(GST_ELEMENT(mPlaybin),
										mRate,
										format,
										flags,
										GST_SEEK_TYPE_SET, 0,
										GST_SEEK_TYPE_SET, mDurationNanos))
					{
						App::get()->console() << "GStreamer Error: unable to seek." << endl;
					}
				}
				mEOSOccurred = false;
				break;

			default:
				break;
		}
	}

	/*
	// Process our queued buffers.
	// First we manage the output buffers.
	while (g_async_queue_length (mQueueOutputBuffer) > 0) {
		GstBuffer *buf = (GstBuffer *) g_async_queue_pop (mQueueOutputBuffer);
		gst_buffer_unref (buf);
	}
	while (g_async_queue_length (mQueuePrerollOutputBuffer) > 0) {
		GstBuffer *buf = (GstBuffer *) g_async_queue_pop (mQueuePrerollOutputBuffer);
		gst_buffer_unref (buf);
	}

	int inputCount = 0;	
	int totalQueuedPreroll = g_async_queue_length (mQueuePrerollInputBuffer);
	int totalQueuedNormal = g_async_queue_length (mQueueInputBuffer);

	while(g_async_queue_length (mQueuePrerollInputBuffer) > 0) {
		GstGLBuffer *buf = (GstGLBuffer *) g_async_queue_pop (mQueuePrerollInputBuffer);
		// if(inputCount == totalQueuedPreroll - 1) 
		if(inputCount == 0) {
			// We only want to deal with the most recent buffer.
			mTexture = buf->texture;
			mTextureWidth = buf->width;
			mTextureHeight = buf->height;
			g_async_queue_push(mQueuePrerollOutputBuffer, buf);
		} else {
			gst_buffer_unref( (GstBuffer*) buf);
		}		
		inputCount++;
	}

	inputCount = 0;
	while(g_async_queue_length (mQueueInputBuffer) > 0) {
		GstGLBuffer *buf = (GstGLBuffer *) g_async_queue_pop (mQueueInputBuffer);
		// if(inputCount == totalQueuedNormal - 1) {
		if(inputCount == 0) {
			// We only want to deal with the most recent buffer.
			mTexture = buf->texture;
			mTextureWidth = buf->width;
			mTextureHeight = buf->height;
			g_async_queue_push(mQueueOutputBuffer, buf);
		} else {
			gst_buffer_unref( (GstBuffer*) buf);
		}
		inputCount++;
	}
	*/

	if(!mEOSOccurred) {
		/*
		// Note: this isn't used now, but it does work. However, it significantly slows down the app's framerate
		//		 since the gst_app_sink_pull_buffer is a blocking call and there could be a lot of video data in there.
		//
		// Poll for the buffer.
		GstGLBuffer *buffer = (GstGLBuffer *) gst_app_sink_pull_buffer(GST_APP_SINK (mAppsink));
		if(buffer) {
			onGstGLTextureUpdateFromBuffer(buffer);
			// Always unreference the buffer once we're done with it.
			gst_buffer_unref( (GstBuffer*) buffer );
		}
		*/

		// App::get()->console() << "GstGLVideoPlayer: texture int: " << mTexture << endl;

		if(mTex) {
			mTex.reset();
		}
		if(mTexture != 0) {
			mTex = shared_ptr<gl::Texture>(new gl::Texture(GL_TEXTURE_RECTANGLE_ARB, mTexture, mTextureWidth, mTextureHeight, true));
		}
	}
}

gl::Texture * GstGLVideoPlayer::getTexture() const
{
	if(mTex) {
		return mTex.get();
	}
	return NULL;
}

void GstGLVideoPlayer::draw()
{
	if(mTexture != 0 && mTex) {
		int winW = App::get()->getWindowWidth();
		int winH = App::get()->getWindowHeight();
		gl::draw( *(mTex.get()), Rectf(0.f, 0.f, (float) winW, (float) winH));
	}
}

/*
	Gets called from the gstOnNewPrerollFromSource and gstOnNewBufferFromSource
	callbacks from appsink. This lets our player know what the latest buffer texture ID
	and width and heights are when it's time to come through for an update.
*/
void GstGLVideoPlayer::onGstGLTextureUpdateFromBuffer(GstGLBuffer * buffer)
{
	mTextureWidth = buffer->width;
	mTextureHeight = buffer->height;
	mTexture = buffer->texture;
}

/*
	This callback is used when working with fakesink and async queues to store
	buffers as they're coming in asynchronously. Not currently used in this implementation.
*/
void GstGLVideoPlayer::onGstGLTextureUpdate()
{
	// Pull the latest off the input queue for processing.
	GstGLBuffer * buffer = (GstGLBuffer*) g_async_queue_pop(mQueueInputBuffer);
	mTextureWidth = buffer->width;
	mTextureHeight = buffer->height;
	mTexture = buffer->texture;

	// Got it, now get this buffering into the output queue for processing.
	g_async_queue_push( mQueueOutputBuffer, buffer );
}

/*
	This callback is used when working with fakesink and async queues to store
	buffers as they're coming in asynchronously. Not currently used in this implementation.
*/
void GstGLVideoPlayer::onGstGLPrerollTextureUpdate()
{
	// Pull the latest off the input queue for processing.
	GstGLBuffer * buffer = (GstGLBuffer*) g_async_queue_pop(mQueuePrerollInputBuffer);
	mTextureWidth = buffer->width;
	mTextureHeight = buffer->height;
	mTexture = buffer->texture;

	// Got it, now get this buffering into the output queue for processing.
	g_async_queue_push( mQueuePrerollOutputBuffer, buffer );
}
