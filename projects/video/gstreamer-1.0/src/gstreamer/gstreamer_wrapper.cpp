#include "stdafx.h"

#include "gstreamer_wrapper.h"

#include <algorithm>
#include <iostream>
#include "ds/debug/logger.h"

#include "gst/net/gstnetclientclock.h"

/// if this is defined, will create and copy video buffers from gstreamer using memcopy
/// undefined will use the buffers directly in creating the textures
#define BUFFERS_COPIED 1

namespace gstwrapper {

GStreamerWrapper::GStreamerWrapper()
  : mFileIsOpen(false)
  , mVideoBuffer(NULL)
  , mAudioBuffer(NULL)
  , mGstPipeline(NULL)
  , mGstVideoSink(NULL)
  , mGstAudioSink(NULL)
  , mGstPanorama(NULL)
  , mGstBus(NULL)
  , mAudioBufferWanted(false)
  , mStartPlaying(true)
  , mStopOnLoopComplete(false)
  , mCustomPipeline(false)
  , mVideoLock(mVideoMutex, std::defer_lock)
  , mVideoBufferSize(0)
  , mClockProvider(NULL)
  , mNetClock(NULL)
  , mBaseTime(0)
  , mRunningTime(0)
  , mPlayFromPause(false)
  , mSeekTime(0)
  , mNewLoop(false)
  , mLivePipeline(false)
  , mFullPipeline(false)
  , mAutoRestartStream(true)
  , mServer(true)
  , mValidInstall(true)
  , mSyncedMode(false)
  , mStreamNeedsRestart(false) {

	mCurrentPlayState = NOT_INITIALIZED;
}

GStreamerWrapper::~GStreamerWrapper() {
	close();

	if (mVideoLock.owns_lock()) {
		try {
			mVideoLock.unlock();
		} catch (std::exception& ex) {
			std::cout << "A fatal deadlock occurred and I can't survive from this one :(" << std::endl
					  << "Probably your screen is stuck and this is the last log line you are reading." << std::endl
					  << "Exception: " << ex.what() << std::endl;
		}
	}
}

void GStreamerWrapper::resetProperties() {
	// init property variables
	mNumVideoStreams	= 0;
	mNumAudioStreams	= 0;
	mCurrentVideoStream = 0;
	mCurrentAudioStream = 0;
	mWidth				= 0;
	mHeight				= 0;
	mCurrentFrameNumber = 0;  // set to invalid, as it is not decoded yet
	mCurrentTimeInMs	= 0;  // set to invalid, as it is not decoded yet
	mIsAudioSigned		= false;
	mIsNewVideoFrame	= false;
	mNumAudioChannels   = 0;
	mAudioSampleRate	= 0;
	mAudioBufferSize	= 0;
	mAudioWidth			= 0;
	mFps				= 0;
	mDurationInMs		= 0;
	mNumberOfFrames		= 0;
	mVolume				= 1.0f;
	mPan				= 0.0f;
	mSpeed				= 1.0f;
	mPlayDirection		= FORWARD;
	mCurrentPlayState   = NOT_INITIALIZED;
	mCurrentGstState	= STATE_NULL;
	mLoopMode			= LOOP;
	mPendingSeek		= false;
	mVideoBufferSize	= 0;
	mLivePipeline		= false;
	mFullPipeline		= false;
	mAutoRestartStream  = true;
	mDurationInNs		= -1;
	mCurrentTimeInNs	= -1;
	mSyncedMode			= false;
	mStreamNeedsRestart = false;
	mStreamingLatency   = 200000000;
}

void GStreamerWrapper::parseFilename(const std::string& theFile) {
	std::string strFilename = theFile;
	std::replace(strFilename.begin(), strFilename.end(), '\\', '/');
	// Check and re-arrange filename string
	if (strFilename.find("file:/", 0) == std::string::npos && strFilename.find("file:///", 0) == std::string::npos &&
		strFilename.find("http://", 0) == std::string::npos) {
		strFilename = "file:///" + strFilename;
	}
	mFilename = strFilename;
}

void GStreamerWrapper::enforceModFourWidth(const int vidWidth, const int vidHeight) {
	int videoWidth  = vidWidth;
	int videoHeight = vidHeight;

	if (videoWidth % 4 != 0) {
		videoWidth += 4 - videoWidth % 4;
	}
	mWidth  = videoWidth;
	mHeight = videoHeight;
}


void GStreamerWrapper::enforceModEightWidth(const int vidWidth, const int vidHeight) {
	int videoWidth  = vidWidth;
	int videoHeight = vidHeight;

	if (videoWidth % 8 != 0) {
		videoWidth += 8 - videoWidth % 8;
	}

	if (videoHeight % 4 != 0) {
		videoHeight += 4 - videoHeight % 4;
	}

	mWidth  = videoWidth;
	mHeight = videoHeight;
}

guint64 GStreamerWrapper::getNetClockTime() {
	if (mNetClock) {
		return gst_clock_get_time(mNetClock);
	}

	return GST_CLOCK_TIME_NONE;
}


bool GStreamerWrapper::isPlayFromPause() { return mPlayFromPause; }

void GStreamerWrapper::clearPlayFromPause() { mPlayFromPause = false; }

bool GStreamerWrapper::isNewLoop() { return mNewLoop; }

void GStreamerWrapper::clearNewLoop() { mNewLoop = false; }

void* GStreamerWrapper::getElementByName(const std::string& gst_element_name) {
	if (mGstPipeline) {
		return gst_bin_get_by_name(GST_BIN(mGstPipeline), gst_element_name.c_str());
	}

	return NULL;
}


static void deinterleave_new_pad(GstElement* element, GstPad* pad, gpointer data) {
	gchar* padName = gst_pad_get_name(pad);
	std::cout << "New pad created! " << padName << std::endl;
	g_free(padName);
}

bool GStreamerWrapper::open(const std::string& strFilename, const bool bGenerateVideoBuffer, const bool bGenerateAudioBuffer,
							const int colorSpace, const int videoWidth, const int videoHeight, const bool hasAudioTrack,
							const double secondsDuration) {
	if (!mValidInstall) {
		return false;
	}

	resetProperties();

	if (secondsDuration > -1) {
		mDurationInNs = gint64(secondsDuration * 1000000 * 1000);
	}

	if (mFileIsOpen) {
		stop();
		close();
	}

	parseFilename(strFilename);

	if (colorSpace == kColorSpaceI420) {
		enforceModEightWidth(videoWidth, videoHeight);
	} else {
		enforceModFourWidth(videoWidth, videoHeight);
	}

	// PIPELINE
	// Init main pipeline --> playbin
	mGstPipeline = gst_element_factory_make("playbin", "pipeline");

	// BUS
	// Set GstBus
	mGstBus = gst_pipeline_get_bus(GST_PIPELINE(mGstPipeline));

	// Open Uri
	g_object_set(mGstPipeline, "uri", mFilename.c_str(), NULL);

	// VIDEO SINK
	// Extract and Config Video Sink
	if (bGenerateVideoBuffer) {
		// Create the video appsink and configure it
		mGstVideoSink = gst_element_factory_make("appsink", "videosink");

		// gst_app_sink_set_max_buffers( GST_APP_SINK( mGstVideoSink ), 2 );
		// gst_app_sink_set_drop( GST_APP_SINK( mGstVideoSink ), true );
		gst_base_sink_set_qos_enabled(GST_BASE_SINK(mGstVideoSink), true);
		gst_base_sink_set_max_lateness(GST_BASE_SINK(mGstVideoSink),
									   -1);  // 1000000000 = 1 second, 40000000 = 40 ms, 20000000 = 20 ms

		// Set some fix caps for the video sink
		GstCaps* caps;

		if (colorSpace == kColorSpaceTransparent) {
			mVideoBufferSize = 4 * mWidth * mHeight;
			caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGRA", "width", G_TYPE_INT, mWidth, "height",
									   G_TYPE_INT, mHeight, NULL);

		} else if (colorSpace == kColorSpaceSolid) {
			mVideoBufferSize = 3 * mWidth * mHeight;

			caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGR", "width", G_TYPE_INT, mWidth, "height",
									   G_TYPE_INT, mHeight, NULL);

		} else if (colorSpace == kColorSpaceI420) {
			// 1.5 * w * h, for I420 color space, which has a full-size luma channel, and 1/4 size U and V color channels
			mVideoBufferSize = (int)(1.5 * mWidth * mHeight);


			caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, mWidth, "height",
									   G_TYPE_INT, mHeight, NULL);
		}

#ifdef BUFFERS_COPIED
		mVideoBuffer = new unsigned char[mVideoBufferSize];
#endif

		gst_app_sink_set_caps(GST_APP_SINK(mGstVideoSink), caps);
		gst_caps_unref(caps);

		// Set the configured video appsink to the main pipeline
		g_object_set(mGstPipeline, "video-sink", mGstVideoSink, (void*)NULL);

		// Tell the video appsink that it should not emit signals as the buffer retrieving is handled via callback methods
		g_object_set(mGstVideoSink, "emit-signals", false, "sync", true, "async", true, (void*)NULL);

		// Set Video Sink callback methods
		mGstVideoSinkCallbacks.eos		   = &GStreamerWrapper::onEosFromVideoSource;
		mGstVideoSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromVideoSource;
		mGstVideoSinkCallbacks.new_sample  = &GStreamerWrapper::onNewBufferFromVideoSource;

		if (mAudioBufferWanted) {
			mGstVideoSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromAudioSource;
			mGstVideoSinkCallbacks.new_sample  = &GStreamerWrapper::onNewBufferFromAudioSource;
		}

		gst_app_sink_set_callbacks(GST_APP_SINK(mGstVideoSink), &mGstVideoSinkCallbacks, this, NULL);

	} else {

		if (mHeight > 0 && mWidth > 0) {
			DS_LOG_VERBOSE(1, "Video size not detected or video buffer not set to be created. Ignoring video output.");

			GstElement* videoSink = gst_element_factory_make("faksesink", NULL);
			g_object_set(mGstPipeline, "video-sink", videoSink, NULL);
		}
	}

	// AUDIO SINK
	// Extract and config Audio Sink
#ifdef _WIN32
	if (!mAudioDevices.empty()) {
		GstElement* bin			 = gst_bin_new("converter_sink_bin");
		GstElement* mainConvert  = gst_element_factory_make("audioconvert", NULL);
		GstElement* mainResample = gst_element_factory_make("audioresample", NULL);
		GstElement* mainVolume   = gst_element_factory_make("volume", "mainvolume");
		GstElement* mainTee		 = gst_element_factory_make("tee", NULL);

		gst_bin_add_many(GST_BIN(bin), mainConvert, mainResample, mainVolume, mainTee, NULL);
		gboolean link_ok = gst_element_link_many(mainConvert, mainResample, mainVolume, mainTee, NULL);
		//	link_ok = gst_element_link_filtered(mainConvert, mainResample, caps);

		for (int i = 0; i < mAudioDevices.size(); i++) {

			// auto-detects guid's based on output name
			mAudioDevices[i].initialize();

			if (mAudioDevices[i].mDeviceGuid.empty()) continue;


			/* This is a start of how to implement 5.1, 7.1, or n-channel audio support.
				Basically, you have to deinterleave the channels from the source.
				Deinterleave converts multichannel audio into single mono tracks, which can then be interleaved back into stereo
			outputs Since the deinterleave element only has dynamic pads, you'll need to add a signal callback and connect the
			channels back up to directsoundsink outputs. You'll need to know the number of channels for each video ahead of time
			and how to map that to output devices. Chances are that this will need to be implemented separately from this class.
			But the code is here for reference. GstElement* deinterleave = gst_element_factory_make("deinterleave", NULL);

			GstElement* queueLeft = gst_element_factory_make("queue", NULL);
			GstElement* queueRight = gst_element_factory_make("queue", NULL);

			GstElement* interleave = gst_element_factory_make("interleave", NULL);
			GstElement* thisQueue = gst_element_factory_make("queue", NULL);
			GstElement* thisConvert = gst_element_factory_make("audioconvert", NULL);
			GstElement* thisSink = gst_element_factory_make("directsoundsink", NULL);

			gst_bin_add_many(GST_BIN(bin), thisQueue, deinterleave, NULL); // , queueLeft, queueRight, interleave, thisQueue,
			thisConvert, thisSink, NULL);

			
			GValueArray* va = g_value_array_new(2);
			GValue v = { 0, };
			g_value_set_enum(&v, GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT);
			g_value_array_append(va, &v);
			g_value_reset(&v);
			g_value_set_enum(&v, GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT);
			g_value_array_append(va, &v);
			g_value_reset(&v);
			g_object_set(interleave, "channel-positions", va, NULL);
			g_value_array_free(va);

			g_object_set(thisSink, "device", mAudioDevices[i].mDeviceGuid.c_str(), NULL);


			g_signal_connect(deinterleave, "pad-added", G_CALLBACK(deinterleave_new_pad), NULL);

			GstPadTemplate* tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(mainTee), "src_%u");
			GstPad* teePad = gst_element_request_pad(mainTee, tee_src_pad_template, NULL, NULL);
			GstPad* queueSinkPad = gst_element_get_static_pad(thisQueue, "sink");
			link_ok = gst_pad_link(teePad, queueSinkPad);
			link_ok = gst_element_link_many(thisQueue, deinterleave, NULL);

			*/

			mAudioDevices[i].mVolumeName   = "volume" + std::to_string(i);
			mAudioDevices[i].mPanoramaName = "panorama" + std::to_string(i);

			GstElement* thisQueue	= gst_element_factory_make("queue", NULL);
			GstElement* thisConvert  = gst_element_factory_make("audioconvert", NULL);
			GstElement* thisPanorama = gst_element_factory_make("audiopanorama", mAudioDevices[i].mPanoramaName.c_str());
			GstElement* thisVolume   = gst_element_factory_make("volume", mAudioDevices[i].mVolumeName.c_str());
			GstElement* thisSink	 = gst_element_factory_make("directsoundsink", NULL);
			g_object_set(thisVolume, "volume", mAudioDevices[i].mVolume, NULL);
			g_object_set(thisSink, "device", mAudioDevices[i].mDeviceGuid.c_str(),
						 NULL);  // , "volume", mAudioDevices[i].mVolume, NULL);
			gst_bin_add_many(GST_BIN(bin), thisQueue, thisConvert, thisPanorama, thisVolume, thisSink, NULL);


			link_ok = gst_element_link_many(thisQueue, thisConvert, thisPanorama, thisVolume, thisSink, NULL);
			GstPadTemplate* tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(mainTee), "src_%u");
			GstPad*			teePad				 = gst_element_request_pad(mainTee, tee_src_pad_template, NULL, NULL);
			GstPad*			queue_audio_pad1	 = gst_element_get_static_pad(thisQueue, "sink");
			link_ok								 = gst_pad_link(teePad, queue_audio_pad1);
		}

		GstPad*  pad	   = gst_element_get_static_pad(mainConvert, "sink");
		GstPad*  ghost_pad = gst_ghost_pad_new("sink", pad);
		GstCaps* caps	  = gst_caps_new_simple("audio/x-raw", "channels", G_TYPE_INT, (int)mAudioDevices.size() * 2, "format",
											 G_TYPE_STRING, "S16LE");
		gst_pad_set_caps(pad, caps);
		gst_caps_unref(caps);
		gst_pad_set_active(ghost_pad, TRUE);
		gst_element_add_pad(bin, ghost_pad);

		g_object_set(mGstPipeline, "audio-sink", bin, NULL);
		gst_object_unref(pad);

	} else
#endif

		if (bGenerateAudioBuffer) {
		if (mCustomPipeline) {
			setCustomFunction();
		} else if (hasAudioTrack) {

			/*
			//Add components for sub-pipeline

			mGstConverter = gst_element_factory_make("audioconvert", "convert");
			mGstPanorama = gst_element_factory_make("audiopanorama", "pan");
			mGstAudioSink	= gst_element_factory_make("autoaudiosink", NULL);
			g_object_set(mGstAudioSink, "sync", true, (void*)NULL);

			GstElement* bin = gst_bin_new("converter_sink_bin");

			//Add and Link sub-pipeline components: 'Audio Converter' ---> 'Panorama' ---> 'Audio Sink'
			gst_bin_add_many(GST_BIN(bin), mGstConverter, mGstPanorama, mGstAudioSink, NULL);
			gboolean link_ok = gst_element_link_many(mGstConverter, mGstPanorama, mGstAudioSink, NULL);

			//Set pan value
			g_object_set(mGstPanorama, "panorama", mPan, NULL);

			//Setup pads to connect main 'playbin' pipeline:   'playbin' ---> 'Audio Converter' ---> 'panorama' ---> 'Audio sink'
			GstPad *pad = gst_element_get_static_pad(mGstConverter, "sink");
			GstPad *ghost_pad = gst_ghost_pad_new("sink", pad);
			gst_pad_set_active(ghost_pad, TRUE);
			gst_element_add_pad(bin, ghost_pad);

			//Set 'bin' pipeline as audio sink
			g_object_set(mGstPipeline, "audio-sink", bin, (void*)NULL);

			gst_object_unref(pad);
			*/

			mGstPanorama = gst_element_factory_make("audiopanorama", "pan");
			g_object_set(mGstPanorama, "panorama", mPan, NULL);
			g_object_set(mGstPipeline, "audio-filter", mGstPanorama, NULL);


			GstElement* thisSink = gst_element_factory_make("directsoundsink", NULL);
			g_object_set(mGstPipeline, "audio-sink", thisSink, NULL);
		}
	} else {
		GstElement* thisSink = gst_element_factory_make("directsoundsink", NULL);
		g_object_set(mGstPipeline, "audio-sink", thisSink, NULL);
	}

	if (mGstPipeline) {
		gst_element_set_state(mGstPipeline, GST_STATE_READY);
		gst_element_set_state(mGstPipeline, GST_STATE_PAUSED);

		setTimePositionInMs(0);

		mCurrentPlayState = OPENED;

		if (mStartPlaying) {
			GstStateChangeReturn retrun = gst_element_set_state(mGstPipeline, GST_STATE_PLAYING);
			if (retrun != GST_STATE_CHANGE_FAILURE) {
				mCurrentPlayState = PLAYING;
			} else {
				std::string errorMessage = "Gstreamer Wrapper Failed to play when loading video! ";
				if (mErrorMessageCallback) mErrorMessageCallback(errorMessage);
				DS_LOG_WARNING(errorMessage);
			}
		}
	}


	// TODO: Check if everything was initialized correctly
	// May need conditional checks when creating the buffers.

	// A file has been opened
	mFileIsOpen = true;

	return true;
}

// This is only for streaming setups that use playbin
static void sourceSetupHandler(void* playbin, GstElement* source, gpointer user_data) {
	// this sets the latency of the rtsp source (or other detected source).
	// It may be required to have this value be configurable in the future, but for now, we're hard-coding
	g_object_set(source, "latency", 100);
}

bool GStreamerWrapper::openStream(const std::string& streamingPipeline, const int videoWidth, const int videoHeight,
								  const uint64_t latencyInNs) {
	if (!mValidInstall) {
		return false;
	}

	resetProperties();

	if (mFileIsOpen) {
		stop();
		close();
	}

	if (streamingPipeline.empty()) {
		DS_LOG_WARNING("Streaming pipeline is empty, aborting stream open.");
		return false;
	}
	enforceModFourWidth(videoWidth, videoHeight);
	mStreamPipeline   = streamingPipeline;
	mStreamingLatency = latencyInNs;
	mFullPipeline	 = true;
	mLivePipeline	 = true;
	mContentType	  = VIDEO_AND_AUDIO;

	// If you've constructed a streaming pipeline yourself, there will be '!' characters separating the elements
	// If not, there won't be any !'s. So auto-detect when the URI has been set, and auto-create the pipeline with a playbin
	// element
	if (mStreamPipeline.find("!") == std::string::npos) {

		mGstPipeline = gst_element_factory_make("playbin", "pipeline");

		// This is obsolete
		// g_signal_connect(mGstPipeline, "source-setup", G_CALLBACK(sourceSetupHandler), NULL);

		// Open Uri
		g_object_set(mGstPipeline, "uri", mStreamPipeline.c_str(), "latency", latencyInNs, NULL);

		// VIDEO SINK
		// Extract and Config Video Sink
		// Create the video appsink and configure it
		mGstVideoSink = gst_element_factory_make("appsink", "videosink");

		gst_base_sink_set_qos_enabled(GST_BASE_SINK(mGstVideoSink), true);
		gst_base_sink_set_max_lateness(GST_BASE_SINK(mGstVideoSink),
									   -1);  // 1000000000 = 1 second, 40000000 = 40 ms, 20000000 = 20 ms

		// Set some fix caps for the video sink

		GstCaps* caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, mWidth, "height",
											G_TYPE_INT, mHeight, NULL);


		gst_app_sink_set_caps(GST_APP_SINK(mGstVideoSink), caps);
		gst_caps_unref(caps);

		// Set the configured video appsink to the main pipeline
		g_object_set(mGstPipeline, "video-sink", mGstVideoSink, (void*)NULL);

		GstElement* audioSink = gst_element_factory_make("autoaudiosink", NULL);
		g_object_set(audioSink, "sync", true, (void*)NULL);
		g_object_set(mGstPipeline, "audio-sink", audioSink, NULL);

	} else {

		// VIDEO SINK

		GError* error = nullptr;
		// PIPELINE
		// Init main pipeline --> playbin
		mGstPipeline = gst_parse_launch(streamingPipeline.c_str(), &error);

		mGstVideoSink	 = gst_bin_get_by_name(GST_BIN(mGstPipeline), "appsink0");
		mGstVolumeElement = gst_bin_get_by_name(GST_BIN(mGstPipeline), "volume0");

		if (error) {
			DS_LOG_WARNING("Streaming pipeline error: " << error->message);
		}
	}


	if (!mGstPipeline) {
		DS_LOG_WARNING("Streaming pipeline failed to be created. ");
		return false;
	}

	// Set some fix caps for the video sink
	// 1.5 * w * h, for I420 color space, which has a full-size luma channel, and 1/4 size U and V color channels
	mVideoBufferSize = (int)(1.5 * mWidth * mHeight);

#ifdef BUFFERS_COPIED
	mVideoBuffer	 = new unsigned char[mVideoBufferSize];
#endif

	// Tell the video appsink that it should not emit signals as the buffer retrieving is handled via callback methods
	g_object_set(mGstVideoSink, "emit-signals", false, (void*)NULL);

	// Set Video Sink callback methods
	mGstVideoSinkCallbacks.eos		   = &GStreamerWrapper::onEosFromVideoSource;
	mGstVideoSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromVideoSource;
	mGstVideoSinkCallbacks.new_sample  = &GStreamerWrapper::onNewBufferFromVideoSource;
	gst_app_sink_set_callbacks(GST_APP_SINK(mGstVideoSink), &mGstVideoSinkCallbacks, this, NULL);


	// BUS
	// Set GstBus
	mGstBus = gst_pipeline_get_bus(GST_PIPELINE(mGstPipeline));

	if (mGstPipeline) {
		// We need to stream the file a little bit in order to be able to retrieve information from it
		gst_element_set_state(mGstPipeline, GST_STATE_READY);
		gst_element_set_state(mGstPipeline, GST_STATE_PAUSED);

		// For some reason this is needed in order to gather video information such as size, framerate etc ...
		// GstState state;
		// gst_element_get_state( mGstPipeline, &state, NULL, 20 * GST_SECOND );
		mCurrentPlayState = OPENED;

		if (mStartPlaying) {
			gst_element_set_state(mGstPipeline, GST_STATE_PLAYING);
			mCurrentPlayState = PLAYING;
		}
	}


	// TODO: Check if everything was initialized correctly
	// May need conditional checks when creating the buffers.

	// A file has been opened
	mFileIsOpen = true;

	return true;
}

bool GStreamerWrapper::parseLaunch(const std::string& fullPipeline, const int videoWidth, const int videoHeight,
								   const int colorSpace, const std::string& videoSinkName, const std::string& volumeElementName,
								   const double secondsDuration) {
	if (!mValidInstall) {
		return false;
	}

	mFullPipeline   = true;
	mLivePipeline   = false;
	mStreamPipeline = fullPipeline;

	resetProperties();

	if (secondsDuration > -1) {
		mDurationInNs = gint64(secondsDuration * 1000000 * 1000);
	}

	if (mFileIsOpen) {
		stop();
		close();
	}

	if (colorSpace == kColorSpaceI420) {
		enforceModEightWidth(videoWidth, videoHeight);
	} else {
		enforceModFourWidth(videoWidth, videoHeight);
	}

	// PIPELINE
	mGstPipeline = gst_parse_launch(fullPipeline.c_str(), NULL);

	if (!mGstPipeline) {
		DS_LOG_WARNING("GStreamer pipeline could not be created! Aborting video playback. Check gstreamer install.");
		return false;
	}

	if (colorSpace == kColorSpaceTransparent) {
		mVideoBufferSize = 4 * mWidth * mHeight;
	} else if (colorSpace == kColorSpaceSolid) {
		mVideoBufferSize = 3 * mWidth * mHeight;
	} else if (colorSpace == kColorSpaceI420) {
		// 1.5 * w * h, for I420 color space, which has a full-size luma channel, and 1/4 size U and V color channels
		mVideoBufferSize = (int)(1.5 * mWidth * mHeight);
	}


#ifdef BUFFERS_COPIED
	mVideoBuffer = new unsigned char[mVideoBufferSize];
#endif

	mGstVideoSink	 = gst_bin_get_by_name(GST_BIN(mGstPipeline), videoSinkName.c_str());
	mGstVolumeElement = gst_bin_get_by_name(GST_BIN(mGstPipeline), volumeElementName.c_str());
	mGstPanorama	  = gst_bin_get_by_name(GST_BIN(mGstPipeline), "panorama0");

	// Set Video Sink callback methods
	mGstVideoSinkCallbacks.eos		   = &GStreamerWrapper::onEosFromVideoSource;
	mGstVideoSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromVideoSource;
	mGstVideoSinkCallbacks.new_sample  = &GStreamerWrapper::onNewBufferFromVideoSource;

	if (mAudioBufferWanted) {
		mGstVideoSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromAudioSource;
		mGstVideoSinkCallbacks.new_sample  = &GStreamerWrapper::onNewBufferFromAudioSource;
	}

	g_object_set(mGstVideoSink, "emit-signals", false, "sync", true, "async", true, (void*)NULL);
	gst_app_sink_set_callbacks(GST_APP_SINK(mGstVideoSink), &mGstVideoSinkCallbacks, this, NULL);

	mGstBus = gst_pipeline_get_bus(GST_PIPELINE(mGstPipeline));
	gst_element_set_state(mGstPipeline, GST_STATE_READY);
	gst_element_set_state(mGstPipeline, GST_STATE_PAUSED);

	setTimePositionInMs(0);

	mFileIsOpen		  = true;
	mCurrentPlayState = OPENED;
	mContentType	  = VIDEO_AND_AUDIO;

	if (mStartPlaying) {
		GstStateChangeReturn retrun = gst_element_set_state(mGstPipeline, GST_STATE_PLAYING);
		if (retrun != GST_STATE_CHANGE_FAILURE) {
			mCurrentPlayState = PLAYING;
		}
	}
	return true;
}

void GStreamerWrapper::setStreamingLatency(uint64_t latency_ns) {
	mStreamingLatency = latency_ns;
	if (!mLivePipeline || !mGstPipeline) {
		return;
	}

	g_object_set(mGstPipeline, "latency", mStreamingLatency, NULL);
}

void GStreamerWrapper::setServerNetClock(const bool isServer, const std::string& addr, const int port, std::uint64_t& netClock,
										 std::uint64_t& clockBaseTime) {
	mSyncedMode = true;
	mServer		= true;
	DS_LOG_INFO("Setting IP Address to: " << addr.c_str() << " Port: " << port);
	if (mClockProvider) {
		gst_object_unref(mClockProvider);
		mClockProvider = nullptr;
	}

	// apply pipeline clock to itself, to make sure we're on charge
	auto clock = gst_system_clock_obtain();
	mNetClock  = clock;
	gst_pipeline_use_clock(GST_PIPELINE(mGstPipeline), clock);
	mClockProvider = gst_net_time_provider_new(clock, addr.c_str(), port);
	gst_clock_set_timeout(mNetClock, 10);

	if (!mClockProvider) {
		DS_LOG_WARNING("Could not instantiate the GST server network clock.");
	}

	// get the time for clients to start based on...

	std::uint64_t newTime = getNetClockTime();
	clockBaseTime		  = newTime;


	// When setting up the server clock, we initialize the base clock to it.
	mBaseTime	= clockBaseTime;
	netClock	 = clockBaseTime;
	mCurrentTime = clockBaseTime;
	// reset my clock so it won't advance detached from net
	gst_element_set_start_time(mGstPipeline, GST_CLOCK_TIME_NONE);

	// set the net clock to start ticking from our base time
	setPipelineBaseTime(netClock);
}

void GStreamerWrapper::setClientNetClock(const bool isServer, const std::string& addr, const int port, std::uint64_t& netClock,
										 std::uint64_t& baseTime) {
	mSyncedMode = true;
	mServer		= false;
	DS_LOG_INFO("Setting IP Address to: " << addr.c_str() << " Port: " << port);

	// reset my clock so it won't advance detached from net
	gst_element_set_start_time(mGstPipeline, GST_CLOCK_TIME_NONE);

	// Create client clock synchronized with net clock.  We want it synchronized exactly, so we provide an initial time of '0'.
	mNetClock = gst_net_client_clock_new("net_clock", addr.c_str(), port, 0);
	gst_clock_set_timeout(mNetClock, 10);


	// apply the net clock
	gst_pipeline_use_clock(GST_PIPELINE(mGstPipeline), mNetClock);

	setPipelineBaseTime(baseTime);
	if (!mNetClock) {
		DS_LOG_WARNING("Could not instantiate the GST client network clock.");
	}
}

void GStreamerWrapper::close() {
	// Collect information under locked mutex
	bool hasVideoSink	 = false;
	bool hasPipeline	  = false;
	bool hasGstBus		  = false;
	bool hasClockProvider = false;
	{
		std::lock_guard<std::mutex> lock(mVideoMutex);
		if (mGstVideoSink != nullptr) hasVideoSink = true;
		if (mGstPipeline != nullptr) hasPipeline = true;
		if (mGstBus != nullptr) hasGstBus = true;
		if (mClockProvider != nullptr) hasClockProvider = true;
	}

	// Clear callbacks before closing
	if (hasVideoSink) {
		GstAppSinkCallbacks emptyCallbacks = {NULL, NULL, NULL};
		gst_app_sink_set_callbacks(GST_APP_SINK(mGstVideoSink), &emptyCallbacks, NULL, NULL);
	}

	// GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(mGstPipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");

	// Lock mutex while modifying member variables
	{
		std::lock_guard<std::mutex> lock(mVideoMutex);
		mFileIsOpen		  = false;
		mCurrentPlayState = NOT_INITIALIZED;
		mContentType	  = NONE;
	}

	// Don't hold mutex while stopping, because a
	// callback could still be running...
	stop();

	if (hasPipeline) {
		// gst_element_set_state( mGstPipeline, GST_STATE_NULL );
		gst_object_unref(mGstPipeline);
	}

	if (hasGstBus) gst_object_unref(mGstBus);

	if (hasClockProvider) gst_object_unref(mClockProvider);

	// Cleanup member variables under mutex
	{
		std::lock_guard<std::mutex> lock(mVideoMutex);

		mGstPipeline  = NULL;
		mGstVideoSink = NULL;
		mGstAudioSink = NULL;
		mGstPanorama  = NULL;
		mGstBus		  = NULL;

#ifdef BUFFERS_COPIED
		delete[] mVideoBuffer;
#endif
		mVideoBuffer = NULL;

		delete[] mAudioBuffer;
		mAudioBuffer = NULL;
	}
}

void GStreamerWrapper::update() {
	handleGStMessage();

	if (mStreamNeedsRestart) {
		mStreamRestartCount++;
		// 2 seconds at 60fps, should prolly move to a timed situation
		if (mStreamRestartCount > 120) {
			mStreamNeedsRestart = false;
			openStream(mStreamPipeline, mWidth, mHeight, mStreamingLatency);
		}
	}
}

uint64_t GStreamerWrapper::getPipelineTime() {
	GstClock* clock = gst_pipeline_get_clock(GST_PIPELINE(mGstPipeline));
	uint64_t  now   = gst_clock_get_time(clock);

	return now;
}

void GStreamerWrapper::setPipelineBaseTime(uint64_t base_time) {
	gst_element_set_base_time(mGstPipeline, base_time);
	mBaseTime = base_time;
}


void GStreamerWrapper::play() {
	if (mGstPipeline) {

		// Only seek on play in net mode
		if (mSyncedMode) {
			GstStateChangeReturn gscr = gst_element_set_state(mGstPipeline, GST_STATE_PLAYING);

			if (gscr == GST_STATE_CHANGE_FAILURE) {
				DS_LOG_WARNING("Gst State change failure");
			}

			if (mServer) {
				if (getState() == PAUSED) {
					std::cout << "Playing from pause" << std::endl;
					mPlayFromPause = true;

					uint64_t baseTime = getPipelineTime();
					setPipelineBaseTime(baseTime);
					uint64_t latency = 200000000;
					setSeekTime(mSeekTime + latency);
				}
			}

			setTimePositionInNs(mSeekTime);
		} else {
			if (mCurrentPlayState != PLAYING) {
				GstStateChangeReturn gscr = gst_element_set_state(mGstPipeline, GST_STATE_PLAYING);

				if (gscr == GST_STATE_CHANGE_FAILURE) {
					DS_LOG_WARNING("Gst State change failure");
				}
			}
		}

		mCurrentPlayState = PLAYING;
	}
}

void GStreamerWrapper::stop() {
	bool hasPipeline = false;
	{
		std::lock_guard<std::mutex> lock(mVideoMutex);
		hasPipeline = (mGstPipeline != nullptr);
	}

	if (hasPipeline) {
		// Stop in this context now means a full clearing of the buffers in gstreamer
		gst_element_set_state(mGstPipeline, GST_STATE_NULL);

		std::lock_guard<std::mutex> lock(mVideoMutex);
		mCurrentPlayState = STOPPED;

#ifndef BUFFERS_COPIED
		mVideoBuffer = NULL;
#endif

	}
}

void GStreamerWrapper::pause() {

	if (mGstPipeline) {
		GstStateChangeReturn gscr = gst_element_set_state(mGstPipeline, GST_STATE_PAUSED);

		if (mSyncedMode) {
			if (mServer) {
				mSeekTime = getCurrentTimeInNs();
			}

			setTimePositionInNs(mSeekTime);
		}

		if (gscr == GST_STATE_CHANGE_FAILURE) {
			DS_LOG_WARNING("GStreamerWrapper: State change failure trying to pause");
		} else {
			mCurrentPlayState = PAUSED;
		}

	} else {
		DS_LOG_WARNING("GStreamerWrapper: Pipeline doesn't exist when trying to pause video.");
	}
}

void GStreamerWrapper::setCurrentVideoStream(int iCurrentVideoStream) {
	if (mLivePipeline) return;

	if (mCurrentVideoStream != iCurrentVideoStream) {
		if (iCurrentVideoStream >= 0 && iCurrentVideoStream < mNumVideoStreams) {
			mCurrentVideoStream = iCurrentVideoStream;

			g_object_set(mGstPipeline, "current-video", mCurrentVideoStream, NULL);
		}
	}
}

void GStreamerWrapper::setCurrentAudioStream(int iCurrentAudioStream) {
	if (mLivePipeline) return;

	if (mCurrentAudioStream != iCurrentAudioStream) {
		if (iCurrentAudioStream >= 0 && iCurrentAudioStream < mNumAudioStreams) {
			mCurrentAudioStream = iCurrentAudioStream;

			g_object_set(mGstPipeline, "current-audio", mCurrentAudioStream, NULL);
		}
	}
}

void GStreamerWrapper::setSpeed(float fSpeed) {
	if (mLivePipeline) return;

	if (fSpeed != mSpeed) {
		mSpeed = fSpeed;
		if (mSpeed < 0.0f) mSpeed = 0.0f;

		changeSpeedAndDirection(mSpeed, mPlayDirection);
	}
}

void GStreamerWrapper::setDirection(PlayDirection direction) {
	if (mLivePipeline) return;

	if (mPlayDirection != direction) {
		mPlayDirection = direction;
		changeSpeedAndDirection(mSpeed, mPlayDirection);
	}
}

void GStreamerWrapper::setLoopMode(LoopMode loopMode) {
	if (mLivePipeline) return;

	mLoopMode = loopMode;
}

void GStreamerWrapper::setFramePosition(gint64 iTargetFrameNumber) {
	mCurrentFrameNumber = iTargetFrameNumber;

	setPosition((float)mCurrentFrameNumber / (float)mNumberOfFrames);
}

void GStreamerWrapper::setTimePositionInMs(double dTargetTimeInMs) {
	mCurrentTimeInMs = dTargetTimeInMs;
	seekFrame((gint64)(mCurrentTimeInMs * 1000000));
}

void GStreamerWrapper::setTimePositionInNs(gint64 iTargetTimeInNs) {
	mCurrentTimeInNs = iTargetTimeInNs;
	seekFrame(mCurrentTimeInNs);
}


void GStreamerWrapper::setPosition(double fPos) {
	if (fPos < 0.0) fPos = 0.0;
	else if (fPos > 1.0) fPos = 1.0;


	mCurrentTimeInMs	= fPos * mCurrentTimeInMs;
	mCurrentFrameNumber = (gint64)(fPos * mNumberOfFrames);
	mCurrentTimeInNs	= (gint64)(fPos * mDurationInNs);

	seekFrame(mCurrentTimeInNs);
}


bool GStreamerWrapper::hasVideo() { return mContentType == VIDEO_AND_AUDIO || mContentType == VIDEO; }

bool GStreamerWrapper::hasAudio() { return mContentType == VIDEO_AND_AUDIO || mContentType == AUDIO; }

std::string GStreamerWrapper::getFileName() { return mFilename; }

unsigned char* GStreamerWrapper::getVideo() {
	std::lock_guard<std::mutex> lock(mVideoMutex);
	mIsNewVideoFrame = false;
	return mVideoBuffer;
}

int GStreamerWrapper::getCurrentVideoStream() { return mCurrentVideoStream; }

int GStreamerWrapper::getCurrentAudioStream() { return mCurrentAudioStream; }

int GStreamerWrapper::getNumberOfVideoStreams() { return mNumVideoStreams; }

int GStreamerWrapper::getNumberOfAudioStreams() { return mNumAudioStreams; }

unsigned int GStreamerWrapper::getWidth() { return mWidth; }

unsigned int GStreamerWrapper::getHeight() { return mHeight; }

bool GStreamerWrapper::isNewVideoFrame() { return mIsNewVideoFrame; }

float GStreamerWrapper::getFps() { return mFps; }

float GStreamerWrapper::getSpeed() { return mSpeed; }

double GStreamerWrapper::getPosition() const {
	return static_cast<double>(getCurrentTimeInNs()) / static_cast<double>(mDurationInNs);
}

gint64 GStreamerWrapper::getCurrentFrameNumber() {
	mCurrentFrameNumber = (gint64)(floor((double)getCurrentTimeInMs() / 1000.0 * mFps));
	return mCurrentFrameNumber;
}

gint64 GStreamerWrapper::getNumberOfFrames() { return mNumberOfFrames; }

double GStreamerWrapper::getCurrentTimeInMs() { return (double)(getCurrentTimeInNs() / 1000000); }

double GStreamerWrapper::getDurationInMs() const { return mDurationInMs; }

gint64 GStreamerWrapper::getCurrentTimeInNs() const {
	GstFormat gstFormat = GST_FORMAT_TIME;
	gst_element_query_position(GST_ELEMENT(mGstPipeline), gstFormat, &mCurrentTimeInNs);
	return mCurrentTimeInNs;
}

gint64 GStreamerWrapper::getDurationInNs() { return mDurationInNs; }

PlayState GStreamerWrapper::getState() const { return mCurrentPlayState; }

PlayDirection GStreamerWrapper::getDirection() { return mPlayDirection; }

LoopMode GStreamerWrapper::getLoopMode() { return mLoopMode; }

ContentType GStreamerWrapper::getContentType() { return mContentType; }

void GStreamerWrapper::setVolume(float fVolume) {
	if (mVolume != fVolume) {
		mVolume = fVolume;
		if (mVolume < 0.0f)
			mVolume = 0.0f;
		else if (mVolume > 1.0f)
			mVolume = 1.0f;

		if (mGstPipeline) g_object_set(mGstPipeline, "volume", mVolume, NULL);

		if (!mAudioDevices.empty()) {
			auto mainVolumeElement = gst_bin_get_by_name(GST_BIN(mGstPipeline), "mainvolume");
			if (mainVolumeElement) {
				g_object_set(mainVolumeElement, "volume", mVolume, NULL);
			}
		}
	}
}

void GStreamerWrapper::setPan(float fPan) {
	if (mPan != fPan && mGstPanorama) {
		mPan = fPan;
		if (mPan < -1.0f)
			mPan = -1.0f;
		else if (mPan > 1.0f)
			mPan = 1.0f;

		g_object_set(mGstPanorama, "panorama", mPan, NULL);
	}
}

unsigned char* GStreamerWrapper::getAudio() {
	std::lock_guard<decltype(mVideoLock)> lock(mVideoLock);
	return mAudioBuffer;
}

bool GStreamerWrapper::getIsAudioSigned() { return mIsAudioSigned; }

int GStreamerWrapper::getNumOfAudioChannels() { return mNumAudioChannels; }

int GStreamerWrapper::getAudioSampleRate() { return mAudioSampleRate; }

size_t GStreamerWrapper::getAudioBufferSize() { return mAudioBufferSize; }

int GStreamerWrapper::getAudioDecodeBufferSize() { return mAudioDecodeBufferSize; }

int GStreamerWrapper::getAudioWidth() { return mAudioWidth; }

float GStreamerWrapper::getCurrentVolume() { return mVolume; }

gint64 GStreamerWrapper::getBaseTime() { return mBaseTime; }


void GStreamerWrapper::setSeekTime(uint64_t seek_time) { mSeekTime = seek_time; }


gint64 GStreamerWrapper::getSeekTime() { return mSeekTime; }


gint64 GStreamerWrapper::getStartTime() { return mStartTime; }

void GStreamerWrapper::setStartTime(uint64_t start_time) { mStartTime = start_time; }

bool GStreamerWrapper::seekFrame(gint64 iTargetTimeInNs) {
	if (mDurationInNs < 0 || mCurrentGstState == STATE_NULL) {
		mPendingSeekTime = iTargetTimeInNs;
		mPendingSeek	 = true;
		return false;
	}

	GstFormat gstFormat = GST_FORMAT_TIME;

	// The flags determine how the seek behaves, in this case we simply want to jump to certain part in stream
	// while keeping the pre-set speed and play direction
	GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH);


	gboolean bIsSeekSuccessful = false;

	if (mSyncedMode && mServer) {
		uint64_t baseTime = getPipelineTime();
		setPipelineBaseTime(baseTime);
		mSeekTime = iTargetTimeInNs;
	}

	if (mPlayDirection == FORWARD) {
		bIsSeekSuccessful = gst_element_seek(GST_ELEMENT(mGstPipeline), mSpeed, gstFormat, gstSeekFlags, GST_SEEK_TYPE_SET,
											 iTargetTimeInNs, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	} else if (mPlayDirection == BACKWARD) {
		bIsSeekSuccessful = gst_element_seek(GST_ELEMENT(mGstPipeline), -mSpeed, gstFormat, gstSeekFlags, GST_SEEK_TYPE_SET, 0,
											 GST_SEEK_TYPE_SET, iTargetTimeInNs);
	}

	if (!(bIsSeekSuccessful == 0)) {
		mPendingSeek = false;
	}

	return bIsSeekSuccessful != 0;
}

bool GStreamerWrapper::changeSpeedAndDirection(float fSpeed, PlayDirection direction) {
	GstFormat gstFormat = GST_FORMAT_TIME;
	// The flags determine how the seek behaves, in this case we stay at the current position in the stream but simply
	// want to change either speed, play direction or both
	GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_SKIP | GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH);

	gboolean bIsSeekSuccessful = false;

	if (direction == FORWARD) {
		bIsSeekSuccessful = gst_element_seek(GST_ELEMENT(mGstPipeline), fSpeed, gstFormat, gstSeekFlags, GST_SEEK_TYPE_SET,
											 getCurrentTimeInNs(), GST_SEEK_TYPE_SET, mDurationInNs);
	} else if (direction == BACKWARD) {
		bIsSeekSuccessful = gst_element_seek(GST_ELEMENT(mGstPipeline), -fSpeed, gstFormat, gstSeekFlags, GST_SEEK_TYPE_SET, 0,
											 GST_SEEK_TYPE_SET, getCurrentTimeInNs());
	}

	return bIsSeekSuccessful != 0;
}

void GStreamerWrapper::retrieveVideoInfo() {
	if (mLivePipeline || mFullPipeline) {
		return;  // streaming sets it's open values
	}
	////////////////////////////////////////////////////////////////////////// Media Duration
	// Nanoseconds
	GstFormat gstFormat = GST_FORMAT_TIME;
	gst_element_query_duration(GST_ELEMENT(mGstPipeline), gstFormat, &mDurationInNs);

	// Milliseconds
	mDurationInMs = (double)(GST_TIME_AS_MSECONDS(mDurationInNs));

	////////////////////////////////////////////////////////////////////////// Stream Info
	// Number of Video Streams
	g_object_get(mGstPipeline, "n-video", &mNumVideoStreams, NULL);

	// Number of Audio Streams
	g_object_get(mGstPipeline, "n-audio", &mNumAudioStreams, NULL);

	// Set Content Type according to the number of available Video and Audio streams
	if (mNumVideoStreams > 0 && mNumAudioStreams > 0) {
		mContentType = VIDEO_AND_AUDIO;
	} else if (mNumVideoStreams > 0) {
		mContentType = VIDEO;
	} else if (mNumAudioStreams > 0) {
		mContentType = AUDIO;
	}

	DS_LOG_VERBOSE(1, "Got video info, duration=" << mDurationInNs << " Number of video streams: " << mNumVideoStreams
												  << " audio: " << mNumAudioStreams);
}

static void print_one_tag(const GstTagList* list, const gchar* tag, gpointer user_data) {
	int i, num;

	num = gst_tag_list_get_tag_size(list, tag);

	for (i = 0; i < num; ++i) {
		const GValue* val;

		/* Note: when looking for specific tags, use the gst_tag_list_get_xyz() API,
		 * we only use the GValue approach here because it is more generic */
		val = gst_tag_list_get_value_index(list, tag, i);

		if (G_VALUE_HOLDS_STRING(val)) {
			std::cout << tag << " " << g_value_get_string(val) << std::endl;
		} else if (G_VALUE_HOLDS_UINT(val)) {
			std::cout << tag << " " << g_value_get_uint(val) << std::endl;
		} else if (G_VALUE_HOLDS_DOUBLE(val)) {
			std::cout << tag << " " << g_value_get_double(val) << std::endl;
		} else if (G_VALUE_HOLDS_BOOLEAN(val)) {
			std::cout << tag << " " << g_value_get_boolean(val) << std::endl;
		}
	}
}

void GStreamerWrapper::setAudioDeviceVolume(ds::GstAudioDevice& theDevice) {
	for (auto it : mAudioDevices) {
		if (it.mDeviceName == theDevice.mDeviceName) {
			auto volumeElement = gst_bin_get_by_name(GST_BIN(mGstPipeline), it.mVolumeName.c_str());
			if (volumeElement) {
				g_object_set(volumeElement, "volume", theDevice.mVolume, (void*)NULL);
			}
			break;
		}
	}
}

void GStreamerWrapper::setAudioDevicePan(ds::GstAudioDevice& theDevice) {
	for (auto it : mAudioDevices) {
		if (it.mDeviceName == theDevice.mDeviceName) {
			auto panElement = gst_bin_get_by_name(GST_BIN(mGstPipeline), it.mPanoramaName.c_str());
			if (panElement) {
				g_object_set(panElement, "panorama", theDevice.mPan, (void*)NULL);
			}
			break;
		}
	}
}

void GStreamerWrapper::handleGStMessage() {
	if (mGstBus) {
		while (gst_bus_have_pending(mGstBus)) {
			mGstMessage = gst_bus_pop(mGstBus);

			GstTagList* tags = NULL;
			if (mGstMessage) {

				switch (GST_MESSAGE_TYPE(mGstMessage)) {

					case GST_MESSAGE_QOS: {
						guint64   processed;
						guint64   dropped;
						GstFormat format = GST_FORMAT_TIME;
						gst_message_parse_qos_stats(mGstMessage, &format, &processed, &dropped);

						DS_LOG_VERBOSE(
							3, "GstWrapper QoS message, seconds processed: " << processed << " frames dropped:" << dropped);

					} break;

					case GST_MESSAGE_WARNING: {
						GError* err;
						gchar*  debug;
						gst_message_parse_warning(mGstMessage, &err, &debug);
						DS_LOG_WARNING("Gst warning: " << err->message << " " << debug);
					} break;
					case GST_MESSAGE_INFO: {
						GError* err;
						gchar*  debug;
						gst_message_parse_info(mGstMessage, &err, &debug);

						DS_LOG_VERBOSE(2, "Gst info: " << err->message << " " << debug);

					} break;

					case GST_MESSAGE_ERROR: {
						GError* err;
						gchar*  debug;
						gst_message_parse_error(mGstMessage, &err, &debug);

						std::stringstream errorMessage;
						errorMessage << "Gst error: Embedded video playback halted: module "
									 << gst_element_get_name(GST_MESSAGE_SRC(mGstMessage)) << " reported " << err->message;

						std::string errorMessageStr = errorMessage.str();
						DS_LOG_ERROR(errorMessageStr << " Debug: " << debug);
						if (mErrorMessageCallback) {
							mErrorMessageCallback(errorMessageStr);
						}

						close();

						if (mFullPipeline && mAutoRestartStream) {
							mStreamNeedsRestart = true;
							mStreamRestartCount = 0;
						}

						g_error_free(err);
						g_free(debug);
					} break;

					case GST_MESSAGE_STATE_CHANGED: {
						GstState oldState;
						GstState newState;
						GstState pendingState;
						gst_message_parse_state_changed(mGstMessage, &oldState, &newState, &pendingState);


						if (newState == GST_STATE_PLAYING) {
							mCurrentGstState = STATE_PLAYING;
						} else if (newState == GST_STATE_NULL) {
							mCurrentGstState = STATE_NULL;
						} else if (newState == GST_STATE_PAUSED) {
							mCurrentGstState = STATE_PAUSED;
						} else if (newState == GST_STATE_READY) {
							mCurrentGstState = STATE_READY;
						}

						DS_LOG_VERBOSE(2, "Gst State Change, new state: " << mCurrentGstState);

					}

					break;

					case GST_MESSAGE_ASYNC_DONE: {
						// In certain cases the volume is set before the pipeline is
						// constructed, so doesn't get applied.
						g_object_set(mGstPipeline, "volume", mVolume, NULL);
						retrieveVideoInfo();

						DS_LOG_VERBOSE(3, "Gst Async done");

						if ((mCurrentGstState == STATE_PLAYING || mCurrentGstState == STATE_PAUSED) && mPendingSeek) {
							seekFrame(mPendingSeekTime);
						}
					} break;

					case GST_MESSAGE_NEW_CLOCK: {
						DS_LOG_VERBOSE(3, "Gst New clock");

						// For example on net sync: http://noraisin.net/diary/?p=954
						// also: #include "gst/net/gstnettimeprovider.h"

					} break;

					case GST_MESSAGE_SEGMENT_DONE: {
						if (mStopOnLoopComplete) {
							stop();
							mStopOnLoopComplete = false;
						} else {
							gst_element_seek(GST_ELEMENT(mGstPipeline), mSpeed, GST_FORMAT_TIME,
											 (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT), GST_SEEK_TYPE_SET, 0,
											 GST_SEEK_TYPE_SET, mDurationInNs);
						}
					} break;
					case GST_MESSAGE_EOS:

						DS_LOG_VERBOSE(2, "Gst EOS Message");
						switch (mLoopMode) {

							case NO_LOOP:
								pause();
								if (mVideoCompleteCallback) mVideoCompleteCallback(this);
								break;

							case LOOP:
								if (mServer) {
									setSeekTime(0);

									// Update the base time with the value of the pipeline/net clock.
									setPipelineBaseTime(getNetClockTime());

									if (gst_element_seek(GST_ELEMENT(mGstPipeline), mSpeed, GST_FORMAT_TIME,
														 (GstSeekFlags)(GST_SEEK_FLAG_FLUSH),  // | GST_SEEK_FLAG_SEGMENT),
														 GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, mDurationInNs)) {
										play();
										mNewLoop = true;
									} else {
										DS_LOG_WARNING("Looping: Could not Seek to requested location.");
									}
								}
								break;


							case BIDIRECTIONAL_LOOP:
								DS_LOG_WARNING("Gst bi-directional looping not implemented!");
								break;

							default:
								break;
						}
						break;

					case GST_MESSAGE_TAG:
						break;

					default:
						DS_LOG_VERBOSE(2, "Gst Message, Type: " << GST_MESSAGE_TYPE_NAME(mGstMessage));

						break;
				}
			}

			gst_message_unref(mGstMessage);
		}
	}
}


void GStreamerWrapper::onEosFromVideoSource(GstAppSink* appsink, void* listener) {
	// ignore
	// Not handling EOS callbacks creates a crash, but we handle EOS on the bus messages
}

GstFlowReturn GStreamerWrapper::onNewPrerollFromVideoSource(GstAppSink* appsink, void* listener) {
	GstSample* gstVideoSinkBuffer = gst_app_sink_pull_preroll(GST_APP_SINK(appsink));
	((GStreamerWrapper*)listener)->newVideoSinkPrerollCallback(gstVideoSinkBuffer);
	gst_sample_unref(gstVideoSinkBuffer);

	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewPrerollFromAudioSource(GstAppSink* appsink, void* listener) {
	GstSample* gstAudioSinkBuffer = gst_app_sink_pull_preroll(GST_APP_SINK(appsink));
	((GStreamerWrapper*)listener)->newAudioSinkPrerollCallback(gstAudioSinkBuffer);
	gst_sample_unref(gstAudioSinkBuffer);

	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewBufferFromVideoSource(GstAppSink* appsink, void* listener) {
	GstSample* gstVideoSinkBuffer = gst_app_sink_pull_sample(GST_APP_SINK(appsink));
	((GStreamerWrapper*)listener)->newVideoSinkBufferCallback(gstVideoSinkBuffer);
	gst_sample_unref(gstVideoSinkBuffer);
	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewBufferFromAudioSource(GstAppSink* appsink, void* listener) {

	GstSample* gstAudioSinkBuffer = gst_app_sink_pull_sample(GST_APP_SINK(appsink));
	((GStreamerWrapper*)listener)->newAudioSinkBufferCallback(gstAudioSinkBuffer);
	gst_sample_unref(gstAudioSinkBuffer);

	return GST_FLOW_OK;
}

void GStreamerWrapper::newVideoSinkPrerollCallback(GstSample* videoSinkSample) {
	std::lock_guard<std::mutex> lock(mVideoMutex);

#ifdef BUFFERS_COPIED
	if (!mVideoBuffer) return;
#endif

	GstBuffer* buff = gst_sample_get_buffer(videoSinkSample);


	GstMapInfo  map;
	GstMapFlags flags = GST_MAP_READ;
	gst_buffer_map(buff, &map, flags);

	size_t videoBufferSize = map.size;

	// sanity check on buffer size, in case something weird happened.
	// In practice, this can fuck up the look of the video, but it plays and doesn't crash
	if (mVideoBufferSize != videoBufferSize) {

		mVideoBufferSize = videoBufferSize;
#ifdef BUFFERS_COPIED
		delete[] mVideoBuffer;
		mVideoBuffer = new unsigned char[mVideoBufferSize];
#endif
	}

#ifdef BUFFERS_COPIED
	memcpy((unsigned char*)mVideoBuffer, map.data, videoBufferSize);
#else
	mVideoBuffer = map.data;
#endif

	if (!mPendingSeek) mIsNewVideoFrame = true;


	gst_buffer_unmap(buff, &map);
}

void GStreamerWrapper::newVideoSinkBufferCallback(GstSample* videoSinkSample) {
	std::lock_guard<std::mutex> lock(mVideoMutex);

#ifdef BUFFERS_COPIED
	if (!mVideoBuffer) return;
#endif

	GstBuffer*  buff = gst_sample_get_buffer(videoSinkSample);
	GstMapInfo  map;
	GstMapFlags flags = GST_MAP_READ;
	gst_buffer_map(buff, &map, flags);


	size_t videoBufferSize = map.size;

	if (mVideoBufferSize != videoBufferSize) {
		mVideoBufferSize = videoBufferSize;
#ifdef BUFFERS_COPIED
		delete[] mVideoBuffer;
		mVideoBuffer = new unsigned char[mVideoBufferSize];
#endif
	}

#ifdef BUFFERS_COPIED
	memcpy((unsigned char*)mVideoBuffer, map.data, videoBufferSize);
#else 
	mVideoBuffer = map.data;
#endif
	if (!mPendingSeek) mIsNewVideoFrame = true;
	mIsNewVideoFrame = true;
	gst_buffer_unmap(buff, &map);
}

void GStreamerWrapper::newAudioSinkPrerollCallback(GstSample* audioSinkBuffer) {

	std::lock_guard<decltype(mVideoLock)> lock(mVideoLock);

	GstBuffer*  buff = gst_sample_get_buffer(audioSinkBuffer);
	GstMapInfo  map;
	GstMapFlags flags = GST_MAP_READ;
	gst_buffer_map(buff, &map, flags);


	size_t audioBufferSize = map.size;

	if (mAudioBufferSize != audioBufferSize) {
		if (mAudioBuffer) delete[] mAudioBuffer;
		mAudioBufferSize = audioBufferSize;
		mAudioBuffer	 = new unsigned char[mAudioBufferSize];
	}

	memcpy((unsigned char*)mAudioBuffer, map.data, mAudioBufferSize);


	gst_buffer_unmap(buff, &map);
}

void GStreamerWrapper::newAudioSinkBufferCallback(GstSample* audioSinkBuffer) {

	std::lock_guard<decltype(mVideoLock)> lock(mVideoLock);

	GstBuffer*  buff = gst_sample_get_buffer(audioSinkBuffer);
	GstMapInfo  map;
	GstMapFlags flags = GST_MAP_READ;
	gst_buffer_map(buff, &map, flags);

	size_t audioBufferSize = map.size;

	if (mAudioBufferSize != audioBufferSize) {
		if (mAudioBuffer) delete[] mAudioBuffer;
		mAudioBufferSize = audioBufferSize;
		mAudioBuffer	 = new unsigned char[mAudioBufferSize];
	}

	memcpy((unsigned char*)mAudioBuffer, map.data, mAudioBufferSize);

	gst_buffer_unmap(buff, &map);
}

void GStreamerWrapper::setVideoCompleteCallback(const std::function<void(GStreamerWrapper* video)>& func) {
	mVideoCompleteCallback = func;
}

void GStreamerWrapper::setErrorMessageCallback(const std::function<void(const std::string& errMessage)>& func) {
	mErrorMessageCallback = func;
}

};  // namespace gstwrapper
