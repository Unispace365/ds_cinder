#include "gstreamer_wrapper.h"

#include "ds/debug/logger.h"
#include <iostream>
#include <algorithm>

namespace {
static GstGLDisplay*  this_gl_display = NULL;
static GstGLContext* this_context = NULL;
}

//#include "gstreamer-1.0/gst/net/gstnetclientclock.h"

namespace gstwrapper
{


GStreamerWrapper::GStreamerWrapper()
	: m_bFileIsOpen( false )
	, m_cVideoBuffer( NULL )
	, m_cAudioBuffer( NULL )
	, m_GstPipeline( NULL )
	, m_GstVideoSink( NULL )
	, m_GstAudioSink( NULL )
	, m_GstBus( NULL )
	, m_StartPlaying(true)
	, m_StopOnLoopComplete(false)
	, m_CustomPipeline(false)
	, m_VideoLock(m_VideoMutex, std::defer_lock)
	, m_VerboseLogging(false)
	, m_SharedDrawable(false)
	, m_SharedTextureId(0)
{
	gst_init( NULL, NULL );
	m_CurrentPlayState = NOT_INITIALIZED;
}

GStreamerWrapper::~GStreamerWrapper()
{
	close();

	if (m_VideoLock.owns_lock()) {
		try {
			m_VideoLock.unlock();
		} catch (std::exception& ex) {
			std::cout	<< "A fatal deadlock occurred and I can't survive from this one :(" << std::endl
						<< "Probably your screen is stuck and this is the last log line you are reading." << std::endl
						<< "Exception: " << ex.what() << std::endl;
		}
	}
}

gboolean reshapeCallback(void *gl_sink, void *gl_ctx, GLuint width, GLuint height, gpointer data)
{
//	std::cout << "Reshape: width=" << width << " height=" << height << "\n";
	//glViewport(0, 0, width, height);
	return TRUE;
}

gboolean drawCallback(void * gl_sink, GstGLContext * gl_ctx, GstSample* sample, gpointer data)
{

	GstVideoFrame v_frame;
	GstVideoInfo v_info;
	guint texture = 0;
	GstBuffer *buf = gst_sample_get_buffer(sample);
	GstCaps *caps = gst_sample_get_caps(sample);
	
		gst_video_info_from_caps(&v_info, caps);
	
	if(!gst_video_frame_map(&v_frame, &v_info, buf, (GstMapFlags)(GST_MAP_READ | GST_MAP_GL))) {
		g_warning("Failed to map the video buffer");
		return TRUE;
		
	}
	
	texture = *(guint *)v_frame.data[0];

	GStreamerWrapper* gwrap = (GStreamerWrapper*)(data);
	if(gwrap){
		//std::cout << "What what!!! " << gwrap->getContentType() << std::endl;
		gwrap->setSharedParams(true, texture);
	}

	//std::cout << "Draw callback: " << gl_ctx << " " << sample << " " << texture << std::endl;

	gst_video_frame_unmap(&v_frame);

	
	return TRUE;
}

void GStreamerWrapper::debugGlOpen(){
	GError* err = nullptr;
	std::string pipeline = "uridecodebin uri=file:///c:/test.mp4 ! glimagesink enable-last-sample=true name=glimagesink0";

	m_GstPipeline = gst_parse_launch(pipeline.c_str(), &err); 
	m_GstBus = gst_pipeline_get_bus(GST_PIPELINE(m_GstPipeline));

	m_GstVideoSink = gst_bin_get_by_name(GST_BIN(m_GstPipeline), "glimagesink0");

	HGLRC this_gl_context = 0;
	HDC this_dc = 0;
	this_gl_context = wglGetCurrentContext();
	this_dc = wglGetCurrentDC();
	//wglMakeCurrent(0, 0);
	const gchar* platform = "wgl";
	this_gl_display = gst_gl_display_new();
	this_context = gst_gl_context_new_wrapped(this_gl_display, (guintptr)this_gl_context,	gst_gl_platform_from_string(platform), GST_GL_API_OPENGL);

	//wglMakeCurrent(this_dc, this_gl_context);
	//g_object_set(G_OBJECT(m_GstVideoSink), "other-context", gl_context, NULL);

	g_signal_connect(G_OBJECT(m_GstVideoSink), "client-reshape", G_CALLBACK(reshapeCallback), NULL);
	g_signal_connect(G_OBJECT(m_GstVideoSink), "client-draw", G_CALLBACK(drawCallback), NULL);


	gst_element_set_state(m_GstPipeline, GST_STATE_READY);
	gst_element_set_state(m_GstPipeline, GST_STATE_PAUSED);
	gst_element_set_state(m_GstPipeline, GST_STATE_PLAYING);
	m_bFileIsOpen = true;
	m_CurrentPlayState = PLAYING;
	m_ContentType = VIDEO;
}

void GStreamerWrapper::debugAppsinkShaderColorspaceOpen(){
	GError* err = nullptr;

	//std::string pipeline = "uridecodebin uri=file:///c:/test.mp4 name=dec dec. ! glcolorscale ! appsink qos=true name=appsink0 dec. ! autoaudiosink sync=true qos=true";
//	std::string pipeline = "uridecodebin uri=file:///c:/test.mp4 name=dec dec. ! videoconvert ! appsink qos=true name=appsink0 dec. ! autoaudiosink sync=true qos=true";
//	std::string pipeline = "uridecodebin uri=file:///c:/test.mp4  name=dec dec. ! appsink qos=true name=appsink0 dec. ! autoaudiosink"; // for some reason, audio like this kills the pipeline
	std::stringstream pipeline;
	pipeline << "playbin uri=\"";
	pipeline << m_strFilename << "\"";
 //file:///c:/test.mp4";
	m_GstPipeline = gst_parse_launch(pipeline.str().c_str(), &err);
	m_GstBus = gst_pipeline_get_bus(GST_PIPELINE(m_GstPipeline));
	// appsink  stuff
	int numVideoBuffers = (int)(1.5 * m_iWidth * m_iHeight);
	std::cout << "Num video chars: " << numVideoBuffers << std::endl;
	m_cVideoBuffer = new unsigned char[numVideoBuffers]; // 8 * 1.5 * w * h, for I420 color space
	//m_cVideoBuffer = new unsigned char[(int)(8 * 3 * m_iWidth * m_iHeight)]; // 8 * 1.5 * w * h, for I420 color space

	// Configure app sink
//	m_GstVideoSink = gst_bin_get_by_name(GST_BIN(m_GstPipeline), "appsink0");

	m_GstVideoSink = gst_element_factory_make("appsink", "videosink");
	g_object_set(m_GstPipeline, "video-sink", m_GstVideoSink, (void*)NULL);


	// Set some fix caps for the video sink
	GstCaps* caps = gst_caps_new_simple("video/x-raw",
										"format", G_TYPE_STRING, "I420",
										"width", G_TYPE_INT, m_iWidth,
										"height", G_TYPE_INT, m_iHeight,
										NULL);


	gst_app_sink_set_caps(GST_APP_SINK(m_GstVideoSink), caps);
	gst_caps_unref(caps);

	gst_app_sink_set_max_buffers(GST_APP_SINK(m_GstVideoSink), 0);
	gst_app_sink_set_drop(GST_APP_SINK(m_GstVideoSink), true);
	gst_base_sink_set_qos_enabled(GST_BASE_SINK(m_GstVideoSink), true);
	gst_base_sink_set_max_lateness(GST_BASE_SINK(m_GstVideoSink), -1); // 1000000000 = 1 second, 40000000 = 40 ms, 20000000 = 20 ms

	// Tell the video appsink that it should not emit signals as the buffer retrieving is handled via callback methods
	g_object_set(m_GstVideoSink, "emit-signals", false, "sync", true, "async", true, "qos", true, (void*)NULL);

	// Set Video Sink callback methods
	m_GstVideoSinkCallbacks.eos = &GStreamerWrapper::onEosFromVideoSource;
	m_GstVideoSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromVideoSource;
	m_GstVideoSinkCallbacks.new_sample = &GStreamerWrapper::onNewBufferFromVideoSource;
	gst_app_sink_set_callbacks(GST_APP_SINK(m_GstVideoSink), &m_GstVideoSinkCallbacks, this, NULL);


	gst_element_set_state(m_GstPipeline, GST_STATE_READY);
	gst_element_set_state(m_GstPipeline, GST_STATE_PAUSED);
	gst_element_set_state(m_GstPipeline, GST_STATE_PLAYING);
	m_bFileIsOpen = true;
	m_CurrentPlayState = PLAYING;
	m_ContentType = VIDEO;
}

bool GStreamerWrapper::open( std::string strFilename, bool bGenerateVideoBuffer, bool bGenerateAudioBuffer, bool isTransparent, int videoWidth, int videoHeight)
{
	// init property variables
	m_iNumVideoStreams = 0;
	m_iNumAudioStreams = 0;
	m_iCurrentVideoStream = 0;
	m_iCurrentAudioStream = 0;
	m_iWidth = videoWidth;		
	m_iHeight = videoHeight;
	m_iCurrentFrameNumber = 0;	// set to invalid, as it is not decoded yet
	m_dCurrentTimeInMs = 0;	// set to invalid, as it is not decoded yet
	m_bIsAudioSigned = false;
	m_bIsNewVideoFrame = false;
	m_iNumAudioChannels = 0;
	m_iAudioSampleRate = 0;
	m_iAudioBufferSize = 0;
	m_iAudioWidth = 0;
	m_AudioEndianness = LITTLE_ENDIAN;
	m_fFps = 0;
	m_dDurationInMs = 0;
	m_iNumberOfFrames = 0;
	m_fVolume = 1.0f;
	m_fSpeed = 1.0f;
	m_PlayDirection = FORWARD;
	m_CurrentPlayState = NOT_INITIALIZED;
	m_CurrentGstState = STATE_NULL;
	m_LoopMode = LOOP;
	m_PendingSeek = false;

	if( m_bFileIsOpen )	{
		stop();
		close();
	}

	std::replace(strFilename.begin(), strFilename.end(), '\\', '/');
	// Check and re-arrange filename string
	if(strFilename.find("file:/", 0) == std::string::npos &&
	   strFilename.find("file:///", 0) == std::string::npos &&
	   strFilename.find("http://", 0) == std::string::npos)
	{
		strFilename = "file:///" + strFilename;
	}
	m_strFilename = strFilename;

	if(videoWidth % 4 != 0){
		videoWidth += 4 - videoWidth % 4;
	}
	m_iWidth = videoWidth;
	m_iHeight = videoHeight;

//	debugGlOpen();
	debugAppsinkShaderColorspaceOpen();
	return true;

	if(isTransparent){
		m_cVideoBuffer = new unsigned char[8 * 4 * videoWidth * videoHeight];
	} else {
		m_cVideoBuffer = new unsigned char[8 * 3 * videoWidth * videoHeight];
	}

	// PIPELINE
	// Init main pipeline --> playbin
	m_GstPipeline = gst_element_factory_make( "playbin", "pipeline" );


	// Open Uri
	g_object_set( m_GstPipeline, "uri", strFilename.c_str(), NULL );


	// VIDEO SINK
	// Extract and Config Video Sink
	if ( bGenerateVideoBuffer ){
		// Create the video appsink and configure it
		m_GstVideoSink = gst_element_factory_make("appsink", "videosink");

		//gst_base_sink_set_sync( GST_BASE_SINK( m_GstVideoSink ), true );
		gst_app_sink_set_max_buffers( GST_APP_SINK( m_GstVideoSink ), 0 );
		gst_app_sink_set_drop( GST_APP_SINK( m_GstVideoSink ), true );
		gst_base_sink_set_qos_enabled(GST_BASE_SINK(m_GstVideoSink), true);
	//	gst_base_sink_set_max_lateness(GST_BASE_SINK(m_GstVideoSink), 20000000); // 1000000000 = 1 second, 40000000 = 40 ms, 20000000 = 20 ms
		gst_base_sink_set_max_lateness(GST_BASE_SINK(m_GstVideoSink), -1); // 1000000000 = 1 second, 40000000 = 40 ms, 20000000 = 20 ms

		// Set some fix caps for the video sink
		GstCaps* caps;
		if(isTransparent){
			caps = gst_caps_new_simple( "video/x-raw",
				"format", G_TYPE_STRING, "BGRA",
				"width", G_TYPE_INT, videoWidth,
				"height", G_TYPE_INT, videoHeight,
			NULL );
		} else  {
			caps= gst_caps_new_simple( "video/x-raw",
				"format", G_TYPE_STRING, "BGR",
				"width", G_TYPE_INT, videoWidth,
				"height", G_TYPE_INT, videoHeight,
				NULL );
		}


		gst_app_sink_set_caps( GST_APP_SINK( m_GstVideoSink ), caps );
		gst_caps_unref( caps );

		// Set the configured video appsink to the main pipeline
		g_object_set( m_GstPipeline, "video-sink", m_GstVideoSink, (void*)NULL );

		// Tell the video appsink that it should not emit signals as the buffer retrieving is handled via callback methods
		g_object_set( m_GstVideoSink, "emit-signals", false, "sync", true, "async", true, (void*)NULL );

		// Set Video Sink callback methods
		m_GstVideoSinkCallbacks.eos = &GStreamerWrapper::onEosFromVideoSource;
		m_GstVideoSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromVideoSource;
		m_GstVideoSinkCallbacks.new_sample = &GStreamerWrapper::onNewBufferFromVideoSource;
		gst_app_sink_set_callbacks( GST_APP_SINK( m_GstVideoSink ), &m_GstVideoSinkCallbacks, this, NULL );

	} else {
		//GstElement* videoSinkbi = gst_element_factory_make( "directdrawsink", NULL );
		//if(!bGenerateAudioBuffer)   //Shoud be bGenerateVideoBuffer - Redundant CEE
			DS_LOG_WARNING("Video size not detected or video buffer not set to be created. Ignoring video output.");
		GstElement* videoSink = gst_element_factory_make( "faksesink", NULL );
		g_object_set( m_GstPipeline, "video-sink", videoSink, NULL );

	}

	// AUDIO SINK
	// Extract and config Audio Sink
	if ( bGenerateAudioBuffer ){
		if (m_CustomPipeline){
			setCustomFunction();
		}
		else {
			// Create and configure audio appsink
			m_GstAudioSink = gst_element_factory_make("appsink", "audiosink");
			gst_base_sink_set_sync(GST_BASE_SINK(m_GstAudioSink), true);
			// Set the configured audio appsink to the main pipeline
			g_object_set(m_GstPipeline, "audio-sink", m_GstAudioSink, (void*)NULL);
			// Tell the video appsink that it should not emit signals as the buffer retrieving is handled via callback methods
			g_object_set(m_GstAudioSink, "emit-signals", false, "sync", true, (void*)NULL);
			//Set up converter  to convert to mono if enabled

		// Set Audio Sink callback methods
		m_GstAudioSinkCallbacks.eos = &GStreamerWrapper::onEosFromAudioSource;
		m_GstAudioSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromAudioSource;
		m_GstAudioSinkCallbacks.new_sample = &GStreamerWrapper::onNewBufferFromAudioSource;
		gst_app_sink_set_callbacks(GST_APP_SINK(m_GstAudioSink), &m_GstAudioSinkCallbacks, this, NULL);
		}

	} else {
		GstElement* audioSink = gst_element_factory_make("autoaudiosink", NULL);
		g_object_set ( m_GstPipeline, "audio-sink", audioSink, NULL );
	}

	// BUS
	// Set GstBus
	m_GstBus = gst_pipeline_get_bus( GST_PIPELINE( m_GstPipeline ) );

	if ( m_GstPipeline ){
		// We need to stream the file a little bit in order to be able to retrieve information from it
		gst_element_set_state( m_GstPipeline, GST_STATE_READY );
		gst_element_set_state( m_GstPipeline, GST_STATE_PAUSED );

		// For some reason this is needed in order to gather video information such as size, framerate etc ...
		//GstState state;
		//gst_element_get_state( m_GstPipeline, &state, NULL, 20 * GST_SECOND );
		m_CurrentPlayState = OPENED;

		if(m_StartPlaying){
			gst_element_set_state(m_GstPipeline, GST_STATE_PLAYING);
			m_CurrentPlayState = PLAYING;
		}
	}


	// TODO: Check if everything was initialized correctly
	// May need conditional checks when creating the buffers.

	// A file has been opened
	m_bFileIsOpen = true;

	return true;
}

void GStreamerWrapper::close(){

	m_bFileIsOpen = false;
	m_CurrentPlayState = NOT_INITIALIZED;
	m_ContentType = NONE;

	stop();

	if ( m_GstPipeline ){
		//gst_element_set_state( m_GstPipeline, GST_STATE_NULL );
		gst_object_unref( m_GstPipeline );
		m_GstPipeline = NULL;
		m_GstVideoSink = NULL;
		m_GstAudioSink = NULL;
	}

	if ( m_GstBus ){
		gst_object_unref( m_GstBus );
		m_GstBus = NULL;
	}

	delete [] m_cVideoBuffer;
	m_cVideoBuffer = NULL;

	delete [] m_cAudioBuffer;
	m_cAudioBuffer = NULL;
}

void GStreamerWrapper::update(){
	handleGStMessage();

// 	if(m_GstVideoSink){
// 		GstSample* sample = NULL;
// 		sample = gst_base_sink_get_last_sample(GST_BASE_SINK(m_GstVideoSink));
// 		if(sample){
// 			std::cout << " valid samples? " << std::endl;
// 		}
// 	}
}

void GStreamerWrapper::play(){
	if ( m_GstPipeline ){
		GstStateChangeReturn gscr = gst_element_set_state( m_GstPipeline, GST_STATE_PLAYING );
		if(gscr == GST_STATE_CHANGE_FAILURE){
			DS_LOG_WARNING("Gst State change failure");
		}
		m_CurrentPlayState = PLAYING;
	}
}

void GStreamerWrapper::stop(){
	if ( m_GstPipeline ){
		// Stop in this context now means a full clearing of the buffers in gstreamer
		gst_element_set_state( m_GstPipeline, GST_STATE_NULL );

		m_CurrentPlayState = STOPPED;
	}
}

void GStreamerWrapper::pause(){
	if ( m_GstPipeline ){
		GstStateChangeReturn gscr = gst_element_set_state(m_GstPipeline, GST_STATE_PAUSED);

		if(gscr == GST_STATE_CHANGE_FAILURE){
			DS_LOG_WARNING("GStreamerWrapper: State change failure trying to pause");
		} else {
			m_CurrentPlayState = PAUSED;
		}

	} else {
		DS_LOG_WARNING("GStreamerWrapper: Pipeline doesn't exist when trying to pause video.");
	}
}



void GStreamerWrapper::setCurrentVideoStream( int iCurrentVideoStream ){
	if ( m_iCurrentVideoStream != iCurrentVideoStream )	{
		if ( iCurrentVideoStream >= 0 && iCurrentVideoStream < m_iNumVideoStreams )		{
			m_iCurrentVideoStream = iCurrentVideoStream;

			g_object_set( m_GstPipeline, "current-video", m_iCurrentVideoStream, NULL );
		}
	}
}

void GStreamerWrapper::setCurrentAudioStream( int iCurrentAudioStream ){
	if ( m_iCurrentAudioStream != iCurrentAudioStream )	{
		if ( iCurrentAudioStream >= 0 && iCurrentAudioStream < m_iNumAudioStreams )		{
			m_iCurrentAudioStream = iCurrentAudioStream;

			g_object_set( m_GstPipeline, "current-audio", m_iCurrentAudioStream, NULL );
		}
	}
}

void GStreamerWrapper::setSpeed( float fSpeed ){
	if( fSpeed != m_fSpeed )
	{
		m_fSpeed = fSpeed;
		if ( m_fSpeed < 0.0f )
			m_fSpeed = 0.0f;

		changeSpeedAndDirection( m_fSpeed, m_PlayDirection );
	}
}

void GStreamerWrapper::setDirection( PlayDirection direction ){
	if ( m_PlayDirection != direction )	{
		m_PlayDirection = direction;
		changeSpeedAndDirection( m_fSpeed, m_PlayDirection );
	}
}

void GStreamerWrapper::setLoopMode( LoopMode loopMode ){
	m_LoopMode = loopMode;
}

void GStreamerWrapper::setFramePosition( gint64 iTargetFrameNumber ){

	m_iCurrentFrameNumber = iTargetFrameNumber;

	setPosition( (float)m_iCurrentFrameNumber / (float)m_iNumberOfFrames );
}

void GStreamerWrapper::setTimePositionInMs( double dTargetTimeInMs ){
	m_dCurrentTimeInMs = dTargetTimeInMs;
	seekFrame( (gint64)(m_dCurrentTimeInMs * 1000000) );
}

void GStreamerWrapper::setTimePositionInNs( gint64 iTargetTimeInNs ){
	m_iCurrentTimeInNs = iTargetTimeInNs;
	seekFrame( m_iCurrentTimeInNs );
}

void GStreamerWrapper::setPosition(double fPos){
	if( fPos < 0.0 )
		fPos = 0.0;
	else if( fPos > 1.0 )
		fPos = 1.0;

	m_dCurrentTimeInMs = fPos * m_dCurrentTimeInMs;
	m_iCurrentFrameNumber = (gint64)(fPos * m_iNumberOfFrames);
	m_iCurrentTimeInNs = (gint64)(fPos * m_iDurationInNs);

	seekFrame( m_iCurrentTimeInNs );
}

bool GStreamerWrapper::hasVideo(){
	return m_ContentType == VIDEO_AND_AUDIO || m_ContentType == VIDEO;
}

bool GStreamerWrapper::hasAudio(){
	return m_ContentType == VIDEO_AND_AUDIO || m_ContentType == AUDIO;
}

std::string GStreamerWrapper::getFileName(){
	return m_strFilename;
}

unsigned char* GStreamerWrapper::getVideo(){
	std::lock_guard<decltype(m_VideoLock)> lock(m_VideoLock);
	m_bIsNewVideoFrame = false;
	return m_cVideoBuffer;
}

int GStreamerWrapper::getCurrentVideoStream(){
	return m_iCurrentVideoStream;
}

int GStreamerWrapper::getCurrentAudioStream(){
	return m_iCurrentAudioStream;
}

int GStreamerWrapper::getNumberOfVideoStreams(){
	return m_iNumVideoStreams;
}

int GStreamerWrapper::getNumberOfAudioStreams(){
	return m_iNumAudioStreams;
}

unsigned int GStreamerWrapper::getWidth(){
	return m_iWidth;
}

unsigned int GStreamerWrapper::getHeight(){
	return m_iHeight;
}

bool GStreamerWrapper::isNewVideoFrame(){
	return m_bIsNewVideoFrame;
}

float GStreamerWrapper::getFps(){
	return m_fFps;
}

float GStreamerWrapper::getSpeed(){
	return m_fSpeed;
}

double GStreamerWrapper::getPosition() const {
	return static_cast<double>(getCurrentTimeInNs()) / static_cast<double>(m_iDurationInNs);
}

gint64 GStreamerWrapper::getCurrentFrameNumber(){
	m_iCurrentFrameNumber = (gint64)(floor( (double)getCurrentTimeInMs() / 1000.0 * m_fFps ));
	return m_iCurrentFrameNumber;
}

gint64 GStreamerWrapper::getNumberOfFrames(){
	return m_iNumberOfFrames;
}

double GStreamerWrapper::getCurrentTimeInMs(){
	return (double)(getCurrentTimeInNs() / 1000000);
}

double GStreamerWrapper::getDurationInMs() const {
	return m_dDurationInMs;
}

gint64 GStreamerWrapper::getCurrentTimeInNs() const {
	GstFormat gstFormat = GST_FORMAT_TIME;
	gst_element_query_position( GST_ELEMENT( m_GstPipeline ), gstFormat, &m_iCurrentTimeInNs );
	return m_iCurrentTimeInNs;
}

gint64 GStreamerWrapper::getDurationInNs() {
	return m_iDurationInNs;
}

PlayState GStreamerWrapper::getState() const {
	return m_CurrentPlayState;
}

PlayDirection GStreamerWrapper::getDirection(){
	return m_PlayDirection;
}

LoopMode GStreamerWrapper::getLoopMode(){
	return m_LoopMode;
}

ContentType GStreamerWrapper::getContentType(){
	return m_ContentType;
}

void GStreamerWrapper::setVolume( float fVolume ){
	if ( m_fVolume != fVolume )
	{
		m_fVolume = fVolume;
		if ( m_fVolume < 0.0f )
			m_fVolume = 0.0f;
		else if ( m_fVolume > 1.0f )
			m_fVolume = 1.0f;

		g_object_set( m_GstPipeline, "volume", m_fVolume, NULL );
	}
}

unsigned char* GStreamerWrapper::getAudio(){
	return m_cAudioBuffer;
}

bool GStreamerWrapper::getIsAudioSigned(){
	return m_bIsAudioSigned;
}

int	GStreamerWrapper::getNumOfAudioChannels(){
	return m_iNumAudioChannels;
}

int GStreamerWrapper::getAudioSampleRate(){
	return m_iAudioSampleRate;
}

int GStreamerWrapper::getAudioBufferSize(){
	return m_iAudioBufferSize;
}

int GStreamerWrapper::getAudioDecodeBufferSize(){
	return m_iAudioDecodeBufferSize;
}

int GStreamerWrapper::getAudioWidth(){
	return m_iAudioWidth;
}

float GStreamerWrapper::getCurrentVolume(){
	return m_fVolume;
}

Endianness GStreamerWrapper::getAudioEndianness(){
	return m_AudioEndianness;
}

bool GStreamerWrapper::seekFrame( gint64 iTargetTimeInNs ){
	if(m_CurrentGstState != STATE_PLAYING){
		m_PendingSeekTime = iTargetTimeInNs;
		m_PendingSeek = true;
		return false;
	}

	GstFormat gstFormat = GST_FORMAT_TIME;

	// The flags determine how the seek behaves, in this case we simply want to jump to certain part in stream
	// while keeping the pre-set speed and play direction
	GstSeekFlags gstSeekFlags = ( GstSeekFlags ) ( GST_SEEK_FLAG_FLUSH );

	gboolean bIsSeekSuccessful = false;

	if ( m_PlayDirection == FORWARD ){
		bIsSeekSuccessful = gst_element_seek( GST_ELEMENT( m_GstPipeline ),
			m_fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			iTargetTimeInNs,
			GST_SEEK_TYPE_NONE,
			GST_CLOCK_TIME_NONE );
	} else if ( m_PlayDirection == BACKWARD )	{
		bIsSeekSuccessful = gst_element_seek( GST_ELEMENT( m_GstPipeline ),
			-m_fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			0,
			GST_SEEK_TYPE_SET,
			iTargetTimeInNs );
	}

	if(!(bIsSeekSuccessful == 0)){
		m_PendingSeek = false;
	}

	return bIsSeekSuccessful != 0;
}

bool GStreamerWrapper::changeSpeedAndDirection( float fSpeed, PlayDirection direction ){
	GstFormat gstFormat = GST_FORMAT_TIME;
	// The flags determine how the seek behaves, in this case we stay at the current position in the stream but simply
	// want to change either speed, play direction or both
	GstSeekFlags gstSeekFlags = (GstSeekFlags)( GST_SEEK_FLAG_SKIP | GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH );

	gboolean bIsSeekSuccessful = false;

	if ( direction == FORWARD )
	{
		bIsSeekSuccessful = gst_element_seek( GST_ELEMENT( m_GstPipeline ),
			fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			getCurrentTimeInNs(),
			GST_SEEK_TYPE_SET,
			m_iDurationInNs );
	}
	else if ( direction == BACKWARD )
	{
		bIsSeekSuccessful = gst_element_seek( GST_ELEMENT( m_GstPipeline ),
			-fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			0,
			GST_SEEK_TYPE_SET,
			getCurrentTimeInNs() );
	}

	return bIsSeekSuccessful != 0;
}

void GStreamerWrapper::retrieveVideoInfo(){
	////////////////////////////////////////////////////////////////////////// Media Duration
	// Nanoseconds
	GstFormat gstFormat = GST_FORMAT_TIME;
	gst_element_query_duration( GST_ELEMENT( m_GstPipeline ), gstFormat, &m_iDurationInNs );

	// Milliseconds
	m_dDurationInMs = (double)(GST_TIME_AS_MSECONDS( m_iDurationInNs ));

	////////////////////////////////////////////////////////////////////////// Stream Info
	// Number of Video Streams
	g_object_get( m_GstPipeline, "n-video", &m_iNumVideoStreams, NULL );

	// Number of Audio Streams
	g_object_get( m_GstPipeline, "n-audio", &m_iNumAudioStreams, NULL );

	// Set Content Type according to the number of available Video and Audio streams
	if ( m_iNumVideoStreams > 0 && m_iNumAudioStreams > 0 ){
		m_ContentType = VIDEO_AND_AUDIO;
	} else if ( m_iNumVideoStreams > 0 ){
		m_ContentType = VIDEO;
	} else if ( m_iNumAudioStreams > 0 ){
		m_ContentType = AUDIO;
	}
}

void GStreamerWrapper::handleGStMessage(){
	if ( m_GstBus )	{

		while ( gst_bus_have_pending( m_GstBus ) ){
			m_GstMessage = gst_bus_pop( m_GstBus );

			if ( m_GstMessage )	{

				switch ( GST_MESSAGE_TYPE( m_GstMessage ) )
				{

				case  GST_MESSAGE_QOS:
				{
					guint64 processed;
					guint64 dropped;
					GstFormat format = GST_FORMAT_TIME;
					gst_message_parse_qos_stats(m_GstMessage, &format, &processed, &dropped);
					if(m_VerboseLogging){
						DS_LOG_INFO("Gst QoS message, seconds processed: " << processed << " frames dropped:" << dropped);
					}
				}
				break;

				case GST_MESSAGE_NEED_CONTEXT:
				{
				//	static GstGLDisplay* gl_display = NULL;
					const gchar *context_type;
					GstContext *context = NULL;
					GstGLContext* glContext = NULL;

					gst_message_parse_context_type(m_GstMessage, &context_type);
				//	g_print("got need context %s\n", context_type);

					if(m_VerboseLogging){
						DS_LOG_INFO("Need context message: " << context_type);
					}


					if(g_strcmp0(context_type, GST_GL_DISPLAY_CONTEXT_TYPE) == 0) {
						GstContext *display_context =
							gst_context_new(GST_GL_DISPLAY_CONTEXT_TYPE, TRUE);
						gst_context_set_gl_display(display_context, this_gl_display);
						gst_element_set_context(GST_ELEMENT(m_GstMessage->src), display_context);
					//	return TRUE;
					} else if(g_strcmp0(context_type, "gst.gl.app_context") == 0) {
						GstContext *app_context = gst_context_new("gst.gl.app_context", TRUE);
						GstStructure *s = gst_context_writable_structure(app_context);
						gst_structure_set(s, "context", GST_GL_TYPE_CONTEXT, this_context,
										  NULL);
						gst_element_set_context(GST_ELEMENT(m_GstMessage->src), app_context);
					//	return TRUE;
					}

// 					if(g_strcmp0(context_type, GST_GL_DISPLAY_CONTEXT_TYPE) == 0) {
// 						if(m_VerboseLogging){
// 							DS_LOG_INFO("Setting gst gl context");
// 						}
						//_open_x11();
// 						HGLRC gl_context = 0;
// 						HDC gl_dc = 0;

					//	gl_context = wglGetCurrentContext();
						//gl_dc = wglGetCurrentDC();
						//wglMakeCurrent(0, 0);

					//	if(!gl_display)
							//gl_display = GST_GL_DISPLAY(gst_gl_display_new());

						//glContext = gst_gl_con
						//context = gst_context_new(GST_GL_DISPLAY_CONTEXT_TYPE, TRUE);
						//gst_context_set_gl_display(context, gl_display);

						//gst_element_set_context(GST_ELEMENT(m_GstMessage->src), context);
					//}

					/* Here you'd retrieve the GstGLContext like in [4] and [5] and do a
					* similar thing as above with the display but matching types/names
					* with the "gst.gl.app_context" case in [6] */

				//	if(context) gst_context_unref(context);
				}
				break;

				case GST_MESSAGE_WARNING:
				{
					GError* err;
					gchar* debug;
					gst_message_parse_warning(m_GstMessage, &err, &debug);
					DS_LOG_WARNING("Gst warning: " << err->message << " " << debug);
				}
				break;
				case GST_MESSAGE_INFO:
					{
						GError* err;
						gchar* debug;
						gst_message_parse_info(m_GstMessage, &err, &debug);

						if(m_VerboseLogging){
							DS_LOG_INFO("Gst info: " << err->message << " " << debug);
						}
					}
					break;

				case GST_MESSAGE_ERROR: 
					{
						GError* err;
						gchar* debug;
						gst_message_parse_error(m_GstMessage, &err, &debug);

						DS_LOG_ERROR("Gst error: Embedded video playback halted: module " << gst_element_get_name(GST_MESSAGE_SRC(m_GstMessage)) << " reported " << err->message);

						close();

						g_error_free(err);
						g_free(debug);
					}
					break;

				case GST_MESSAGE_STATE_CHANGED:
					{
						//retrieveVideoInfo();
						GstState oldState;
						GstState newState;
						GstState pendingState;
						gst_message_parse_state_changed(m_GstMessage, &oldState, &newState, &pendingState);

						//std::cout << "State changed: " << oldState << " " << newState << " " << pendingState << std::endl;

						if (newState == GST_STATE_PLAYING) {
							m_CurrentGstState = STATE_PLAYING;
						} else if(newState == GST_STATE_NULL){
							m_CurrentGstState = STATE_NULL;
						} else if(newState == GST_STATE_PAUSED){
							m_CurrentGstState = STATE_PAUSED;
						} else if(newState == GST_STATE_READY){
							m_CurrentGstState = STATE_READY;
						}

						if( (m_CurrentGstState == GST_STATE_PLAYING || m_CurrentGstState == GST_STATE_PAUSED) && m_PendingSeek){
							seekFrame(m_PendingSeekTime);
						}

					  }

				break;

				case GST_MESSAGE_ASYNC_DONE :{
					retrieveVideoInfo();
				}
				break;

				case GST_MESSAGE_NEW_CLOCK :{
					// For example on net sync: http://noraisin.net/diary/?p=954
					// also: #include "gst/net/gstnettimeprovider.h"

					//GstClock* clock = gst_net_client_clock_new("new", "127.0.0.1", 6767, 0);
					//gst_pipeline_use_clock(GST_PIPELINE(m_GstPipeline), clock);
				}
				break;

				case GST_MESSAGE_SEGMENT_DONE : 
					{
						if(m_StopOnLoopComplete){
							stop();
							m_StopOnLoopComplete = false;
						} else {
							gst_element_seek(GST_ELEMENT(m_GstPipeline),
											 m_fSpeed,
											 GST_FORMAT_TIME,
											 (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT),
											 GST_SEEK_TYPE_SET,
											 0,
											 GST_SEEK_TYPE_SET,
											 m_iDurationInNs);

						}
					}
												break;
				case GST_MESSAGE_EOS:

					switch ( m_LoopMode )
					{
						case NO_LOOP:
							pause();
							if(mVideoCompleteCallback) mVideoCompleteCallback(this);
							break;

						case LOOP:
							{

							gst_element_seek( GST_ELEMENT( m_GstPipeline ),
								m_fSpeed,
								GST_FORMAT_TIME,
								(GstSeekFlags)(GST_SEEK_FLAG_FLUSH),// | GST_SEEK_FLAG_SEGMENT),
								GST_SEEK_TYPE_SET,
								0,
								GST_SEEK_TYPE_SET,
								m_iDurationInNs );
							play();

							}

							break;


						case BIDIRECTIONAL_LOOP:
							DS_LOG_WARNING("Gst bi-directional looping not implemented!");
							//m_PlayDirection = (PlayDirection)-m_PlayDirection;
							//stop();
							//play();
							break;

						default:
							break;
					}
					break;

				default:
					if(m_VerboseLogging){
						DS_LOG_INFO("Gst Message, Type: " << GST_MESSAGE_TYPE_NAME(m_GstMessage));
					}
					break;
				}
			}

			gst_message_unref( m_GstMessage );
		}
	}
}

void GStreamerWrapper::onEosFromVideoSource(GstAppSink* appsink, void* listener){
	// ignore
	// Not handling EOS callbacks creates a crash, but we handle EOS on the bus messages
}

void GStreamerWrapper::onEosFromAudioSource(GstAppSink* appsink, void* listener){
	// ignore
	// Not handling EOS callbacks creates a crash, but we handle EOS on the bus messages
}


void GStreamerWrapper::setVerboseLogging(const bool verboseOn){
	m_VerboseLogging = verboseOn;
}


bool GStreamerWrapper::getSharedDrawable(){
	return m_SharedDrawable;
}


int GStreamerWrapper::getSharedTextureId(){
	return m_SharedTextureId;
}

GstFlowReturn GStreamerWrapper::onNewPrerollFromVideoSource(GstAppSink* appsink, void* listener){
	GstSample* gstVideoSinkBuffer = gst_app_sink_pull_preroll( GST_APP_SINK( appsink ) );
	( ( GStreamerWrapper *)listener )->newVideoSinkPrerollCallback( gstVideoSinkBuffer );
	gst_sample_unref( gstVideoSinkBuffer );

	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewPrerollFromAudioSource( GstAppSink* appsink, void* listener ){
	GstSample* gstAudioSinkBuffer = gst_app_sink_pull_preroll( GST_APP_SINK( appsink ) );
	( ( GStreamerWrapper * )listener )->newAudioSinkPrerollCallback( gstAudioSinkBuffer );
	gst_sample_unref( gstAudioSinkBuffer );

	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewBufferFromVideoSource( GstAppSink* appsink, void* listener ){
	//Poco::Timestamp::TimeVal pre = Poco::Timestamp().epochMicroseconds();
	
	GstSample* gstVideoSinkBuffer = gst_app_sink_pull_sample( GST_APP_SINK( appsink ) );

	//Poco::Timestamp::TimeVal mid = Poco::Timestamp().epochMicroseconds();
	
	( ( GStreamerWrapper * )listener )->newVideoSinkBufferCallback( gstVideoSinkBuffer );

	//Poco::Timestamp::TimeVal copied = Poco::Timestamp().epochMicroseconds();
	
	gst_sample_unref( gstVideoSinkBuffer );

// 	Poco::Timestamp::TimeVal post = Poco::Timestamp().epochMicroseconds();
// 	std::cout <<  "mid: " << (float)(mid - pre) / 1000000.0f << " copied: " << (float)(copied - mid) / 1000000.0f << " post: " << (float)(post - copied) / 1000000.0f << std::endl;

	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewBufferFromAudioSource( GstAppSink* appsink, void* listener ){

	GstSample* gstAudioSinkBuffer = gst_app_sink_pull_sample( GST_APP_SINK( appsink ) );
	( ( GStreamerWrapper * )listener )->newAudioSinkBufferCallback( gstAudioSinkBuffer );
	gst_sample_unref( gstAudioSinkBuffer );

	return GST_FLOW_OK;
}

void GStreamerWrapper::newVideoSinkPrerollCallback( GstSample* videoSinkSample ){
	std::lock_guard<decltype(m_VideoLock)> lock(m_VideoLock);
	GstBuffer* buff = gst_sample_get_buffer(videoSinkSample);	


	GstMapInfo map;
	GstMapFlags flags = GST_MAP_READ;
	gst_buffer_map(buff, &map, flags);

	unsigned int videoBufferSize = map.size; //(unsigned int)(gst_buffer_get_size(buff));

	std::cout << "Gst video buffer size: " << videoBufferSize << std::endl;
	// Allocate memory for the video pixels according to the vide appsink buffer size
	if ( m_cVideoBuffer == NULL ){
		if(!m_PendingSeek) m_bIsNewVideoFrame = true;
	}
	
	// Copy the video appsink buffer data to our unsigned char array
	memcpy( (unsigned char *)m_cVideoBuffer, map.data, videoBufferSize );

	gst_buffer_unmap(buff, &map);
}

void GStreamerWrapper::newVideoSinkBufferCallback( GstSample* videoSinkSample ){
	std::lock_guard<decltype(m_VideoLock)> lock(m_VideoLock);
	if(!m_PendingSeek) m_bIsNewVideoFrame = true;

	GstBuffer* buff = gst_sample_get_buffer(videoSinkSample);	
	GstMapInfo map;
	GstMapFlags flags = GST_MAP_READ;
	gst_buffer_map(buff, &map, flags);

	// Copy the video appsink buffer data to our unsigned char array
	memcpy( (unsigned char *)m_cVideoBuffer, map.data, map.size );

	gst_buffer_unmap(buff, &map);
}

void GStreamerWrapper::newAudioSinkPrerollCallback( GstSample* audioSinkBuffer ){
	// NOTE:
	// This is the old implementation from GStreamer 0.10.
	// It's left here in case it's helpful for re-implemenation
	
	//if ( m_cAudioBuffer == NULL )
	//{
	//	m_iAudioBufferSize = GST_BUFFER_SIZE( audioSinkBuffer );
	//	m_cAudioBuffer = new unsigned char[m_iAudioBufferSize];

	//	////////////////////////////////////////////////////////////////////////// AUDIO DATA

	//	/*
	//		Note: For some reason, with this version of GStreamer the only way to retrieve the audio metadata
	//		is to read the caps from the audio appsink buffer and via a GstStructure we can retrieve the needed
	//		values from the caps. After lots of research I also found another possibility by using GstAudioInfo
	//		but this struct is not available in this version.

	//		If a later version of GStreamer is ever compiled in a valid way so it can be used with Visual Studio
	//		it would definitely be a good idea to retrieve the audio information somewhere else in the code.
	//		But this piece of code does it well for now.
	//	*/

	//	// Get Audio metadata
	//	// http://gstreamer.freedesktop.org/data/doc/gstreamer/head/pwg/html/section-types-definitions.html
	//	GstCaps* audioCaps = gst_buffer_get_caps( audioSinkBuffer );
	//	GstStructure* gstStructure = gst_caps_get_structure( audioCaps, 0 );

	//	// Is audio data signed or not?
	//	gboolean isAudioSigned;
	//	gst_structure_get_boolean( gstStructure, "signed", &isAudioSigned );
	//	m_bIsAudioSigned = (bool)(isAudioSigned);

	//	// Number of channels
	//	gst_structure_get_int( gstStructure, "channels", &m_iNumAudioChannels );
	//	// Audio sample rate
	//	gst_structure_get_int( gstStructure, "rate", &m_iAudioSampleRate );
	//	// Audio width
	//	gst_structure_get_int( gstStructure, "width", &m_iAudioWidth );

	//	// Calculate the audio buffer size without the number of channels and audio width
	//	m_iAudioDecodeBufferSize = m_iAudioBufferSize / m_iNumAudioChannels / ( m_iAudioWidth / 8 );

	//	// Audio endianness
	//	gint audioEndianness;
	//	gst_structure_get_int( gstStructure, "endianness",  &audioEndianness );
	//	m_AudioEndianness = (Endianness)audioEndianness;

	//	gst_caps_unref( audioCaps );
	//}
	//else
	//{
	//	// The Audio Buffer size may change during runtime so we keep track if the buffer changes
	//	// If so, delete the old buffer and re-allocate it with the respective new buffer size
	//	int bufferSize = GST_BUFFER_SIZE( audioSinkBuffer );
	//	if ( m_iAudioBufferSize != bufferSize )
	//	{
	//		// Allocate the audio data array according to the audio appsink buffer size
	//		m_iAudioBufferSize = bufferSize;
	//		delete [] m_cAudioBuffer;
	//		m_cAudioBuffer = NULL;

	//		m_cAudioBuffer = new unsigned char[m_iAudioBufferSize];
	//	}
	//}

	//// Copy the audio appsink buffer data to our unsigned char array
	//memcpy( (unsigned char *)m_cAudioBuffer, (unsigned char *)GST_BUFFER_DATA( audioSinkBuffer ), GST_BUFFER_SIZE( audioSinkBuffer ) );
}

void GStreamerWrapper::newAudioSinkBufferCallback( GstSample* audioSinkBuffer )
{

	//---------------- Potential GStreamer 1.0 implementation
// 	GstBuffer *buffer;
// 	GstMapInfo map;
// 	buffer = gst_sample_get_buffer(audioSinkBuffer);
// 
// 	gst_buffer_map(buffer, &map, GST_MAP_READ);
// 
// 	// Here you'll send or copy map.data somewhere
// 	//std::cout << "New Audio Sink Callback: " << map.size << std::endl;
// 
// 	gst_buffer_unmap(buffer, &map);


	//---------------- Below is the old 0.10 implementation
	//// The Audio Buffer size may change during runtime so we keep track if the buffer changes
	//// If so, delete the old buffer and re-allocate it with the respective new buffer size
	//int bufferSize = GST_BUFFER_SIZE( audioSinkBuffer );

	//if ( m_iAudioBufferSize != bufferSize )
	//{
	//	m_iAudioBufferSize = bufferSize;
	//	delete [] m_cAudioBuffer;
	//	m_cAudioBuffer = NULL;

	//	m_cAudioBuffer = new unsigned char[m_iAudioBufferSize];

	//	// Recalculate the audio decode buffer size due to change in buffer size
	//	m_iAudioDecodeBufferSize = m_iAudioBufferSize / m_iNumAudioChannels / ( m_iAudioWidth / 8 );
	//}

	//// Copy the audio appsink buffer data to our unsigned char array
	//memcpy( (unsigned char *)m_cAudioBuffer, (unsigned char *)GST_BUFFER_DATA( audioSinkBuffer ), GST_BUFFER_SIZE( audioSinkBuffer ) );
}

void GStreamerWrapper::setVideoCompleteCallback( const std::function<void(GStreamerWrapper* video)> &func ){
	mVideoCompleteCallback = func;
}

void GStreamerWrapper::setSharedParams(const bool drawable, const int textureId){
	m_SharedTextureId = textureId;
	m_SharedDrawable = drawable;
}


};
