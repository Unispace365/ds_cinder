#include "gstreamer_wrapper.h"

#include "ds/debug/logger.h"
#include <iostream>
#include <algorithm>

#include "gst/net/gstnetclientclock.h"

namespace gstwrapper
{


	GStreamerWrapper::GStreamerWrapper()
		: m_bFileIsOpen(false)
		, m_cVideoBuffer(NULL)
		, m_cAudioBuffer(NULL)
		, m_GstPipeline(NULL)
		, m_GstVideoSink(NULL)
		, m_GstAudioSink(NULL)
		, m_GstBus(NULL)
		, m_StartPlaying(true)
		, m_StopOnLoopComplete(false)
		, m_CustomPipeline(false)
		, m_VideoLock(m_VideoMutex, std::defer_lock)
		//, m_VerboseLogging(true)
		, m_VerboseLogging(false)
		, m_cVideoBufferSize(0)
		, mClockProvider(NULL)
		, m_NetClock(NULL)
		, m_BaseTime(0)
		, m_RunningTime(0)
		, m_playFromPause(false)
		, m_SeekTime(0)
		, m_isFastSeeking(false)
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

void GStreamerWrapper::resetProperties(){
	// init property variables
	m_iNumVideoStreams = 0;
	m_iNumAudioStreams = 0;
	m_iCurrentVideoStream = 0;
	m_iCurrentAudioStream = 0;
	m_iWidth = 0;
	m_iHeight = 0;
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
	m_cVideoBufferSize = 0;
	m_iDurationInNs = -1;
	m_iCurrentTimeInNs = -1;


}

void GStreamerWrapper::parseFilename(const std::string& theFile){
	std::string strFilename = theFile;
	std::replace(strFilename.begin(), strFilename.end(), '\\', '/');
	// Check and re-arrange filename string
	if(strFilename.find("file:/", 0) == std::string::npos &&
	   strFilename.find("file:///", 0) == std::string::npos &&
	   strFilename.find("http://", 0) == std::string::npos)
	{
		strFilename = "file:///" + strFilename;
	}
	m_strFilename = strFilename;
}

void GStreamerWrapper::enforceModFourWidth(const int vidWidth, const int vidHeight){
	int videoWidth = vidWidth;
	int videoHeight = vidHeight;

	if(videoWidth % 4 != 0){
		videoWidth += 4 - videoWidth % 4;
	}
	m_iWidth = videoWidth;
	m_iHeight = videoHeight;
}


guint64 GStreamerWrapper::getNetClockTime()
{
	std::uint64_t newTime = gst_clock_get_time(m_NetClock);
	return newTime;
}


bool GStreamerWrapper::isPlayFromPause()
{
	return m_playFromPause;
}

void GStreamerWrapper::clearPlayFromPause() {
	m_playFromPause = false;
}

bool GStreamerWrapper::open(const std::string& strFilename, const bool bGenerateVideoBuffer, const bool bGenerateAudioBuffer, const int colorSpace, const int videoWidth, const int videoHeight){
	resetProperties();

	if( m_bFileIsOpen )	{
		stop();
		close();
	}

	parseFilename(strFilename);
	enforceModFourWidth(videoWidth, videoHeight);

	// PIPELINE
	// Init main pipeline --> playbin
	m_GstPipeline = gst_element_factory_make( "playbin", "pipeline" );

	// Open Uri
	g_object_set(m_GstPipeline, "uri", m_strFilename.c_str(), NULL);


	// VIDEO SINK
	// Extract and Config Video Sink
	if ( bGenerateVideoBuffer ){
		// Create the video appsink and configure it
		m_GstVideoSink = gst_element_factory_make("appsink", "videosink");

		gst_app_sink_set_max_buffers( GST_APP_SINK( m_GstVideoSink ), 0 );
		gst_app_sink_set_drop( GST_APP_SINK( m_GstVideoSink ), true );
		gst_base_sink_set_qos_enabled(GST_BASE_SINK(m_GstVideoSink), true);
		gst_base_sink_set_max_lateness(GST_BASE_SINK(m_GstVideoSink), -1); // 1000000000 = 1 second, 40000000 = 40 ms, 20000000 = 20 ms

		// Set some fix caps for the video sink
		GstCaps* caps;

		if(colorSpace == kColorSpaceTransparent){
			m_cVideoBufferSize = 4 * m_iWidth * m_iHeight;
			caps = gst_caps_new_simple("video/x-raw",
									   "format", G_TYPE_STRING, "BGRA",
									   "width", G_TYPE_INT, m_iWidth,
									   "height", G_TYPE_INT, m_iHeight,
									   NULL);

		} else if(colorSpace == kColorSpaceSolid){
			m_cVideoBufferSize = 3 * m_iWidth * m_iHeight;

			caps = gst_caps_new_simple("video/x-raw",
									   "format", G_TYPE_STRING, "BGR",
									   "width", G_TYPE_INT, m_iWidth,
									   "height", G_TYPE_INT, m_iHeight,
									   NULL);

		} else if(colorSpace == kColorSpaceI420){
			// 1.5 * w * h, for I420 color space, which has a full-size luma channel, and 1/4 size U and V color channels
			m_cVideoBufferSize = (int)(1.5 * m_iWidth * m_iHeight);


			caps = gst_caps_new_simple("video/x-raw",
									   "format", G_TYPE_STRING, "I420",
									   "width", G_TYPE_INT, m_iWidth,
									   "height", G_TYPE_INT, m_iHeight,
									   NULL);
		}

		m_cVideoBuffer = new unsigned char[m_cVideoBufferSize];

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

		if(m_iHeight > 0 && m_iWidth > 0){
			if(m_VerboseLogging){
				DS_LOG_INFO("Video size not detected or video buffer not set to be created. Ignoring video output.");
			}
			GstElement* videoSink = gst_element_factory_make("faksesink", NULL);
			g_object_set(m_GstPipeline, "video-sink", videoSink, NULL);
		}

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

void GStreamerWrapper::setServerNetClock(const bool isServer, const std::string& addr, const int port, std::uint64_t& netClock, std::uint64_t& clockBaseTime){
	mServer = true;

		if(mClockProvider){
			gst_object_unref(mClockProvider);
			mClockProvider = nullptr; 
		}

		// apply pipeline clock to itself, to make sure we're on charge
		auto clock = gst_system_clock_obtain();
		m_NetClock = clock;
		gst_pipeline_use_clock(GST_PIPELINE(m_GstPipeline), clock);
		mClockProvider = gst_net_time_provider_new(clock, addr.c_str(), port);
		if(!mClockProvider)		{
			DS_LOG_WARNING("Could not instantiate the GST server network clock.");
		}
		// get the time for clients to start based on...

		std::uint64_t newTime = getNetworkTime();
		clockBaseTime = newTime;
		std::cout << "----------- port:" << port << " clock time:" << clockBaseTime << " " << newTime << std::endl;

		//When setting up the server clock, we initialize the base clock to it.
		m_BaseTime = clockBaseTime;
		netClock = clockBaseTime;
		m_CurrentTime = clockBaseTime;
		// reset my clock so it won't advance detached from net
		gst_element_set_start_time(m_GstPipeline, GST_CLOCK_TIME_NONE);

		// set the net clock to start ticking from our base time
		setPipelineBaseTime(netClock);
}

void GStreamerWrapper::setClientNetClock(const bool isServer, const std::string& addr, const int port, std::uint64_t& netClock,  std::uint64_t& baseTime){
	mServer = false;

		//Create new client clock if it doesn't exist
		if (!m_NetClock)
		{
			// reset my clock so it won't advance detached from net
			gst_element_set_start_time(m_GstPipeline, GST_CLOCK_TIME_NONE);

			//Create client clock synchronized with net clock.  We want it synchronized exactly, so we provide an initial time of '0'.
			m_NetClock = gst_net_client_clock_new("net_clock", addr.c_str(), port, 0);

			// apply the net clock
			gst_pipeline_use_clock(GST_PIPELINE(m_GstPipeline), m_NetClock);

			if (!m_NetClock)
			{
				DS_LOG_WARNING("Could not instantiate the GST client network clock.");
			}
		}

		// Update base time if server has updated it's base time.
		if (baseTime != m_BaseTime) 
		{
			setPipelineBaseTime(baseTime);
		}
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

	std::lock_guard<decltype(m_VideoLock)> lock(m_VideoLock);
	delete [] m_cVideoBuffer;
	m_cVideoBuffer = NULL;

	delete [] m_cAudioBuffer;
	m_cAudioBuffer = NULL;

	if(mClockProvider){
		gst_object_unref(mClockProvider);
	}
}


void GStreamerWrapper::update(){
	handleGStMessage();
}

uint64_t GStreamerWrapper::getPipelineTime(){
	GstClock* clock = gst_pipeline_get_clock(GST_PIPELINE(m_GstPipeline));
	uint64_t now = gst_clock_get_time(clock);

	return now;
}

uint64_t GStreamerWrapper::getNetworkTime(){
	uint64_t now = gst_clock_get_time(m_NetClock);
	return now;
}


void GStreamerWrapper::setPipelineBaseTime(uint64_t base_time){
	gst_element_set_base_time(m_GstPipeline, base_time);
	m_BaseTime = base_time;
	std::cout << "------ Changing base time: " << m_BaseTime << std::endl;
}


void GStreamerWrapper::play(){
	if (m_GstPipeline){

		m_isFastSeeking = false;

		if (mServer) 
		{
			std::cout << "++++++++++++++ Server Received command to Play" << std::endl;

			GstStateChangeReturn gscr = gst_element_set_state(m_GstPipeline, GST_STATE_PLAYING);

			if (getState() == PAUSED){

				m_playFromPause = true;

				std::cout << "----------- Playing" << std::endl;
				uint64_t baseTime = getPipelineTime();
				setPipelineBaseTime(baseTime);
				uint64_t tmp = m_SeekTime / (uint64_t)(1000000000);
				std::cout << " Server resuming playback from: " << tmp << "  seconds" << std::endl;

				//Pipeline continues to roll a few frames after pause state is entered.  We need to compensate for it.
				uint64_t latency = 200000000;


				setSeekTime(m_SeekTime + latency);
				seekFrame(m_SeekTime);


			}

			if (gscr == GST_STATE_CHANGE_FAILURE){
				DS_LOG_WARNING("Gst State change failure");
			}
			m_CurrentPlayState = PLAYING;

		}
		else {
			std::cout << "++++++++++++++ Client Received command to Play" << std::endl;

			//if (m_CurrentPlayState == PAUSED) {
			//	//m_playFromPause = true;
			//	std::cout << "play(): Client changing states from Paused to playing." << std::endl;

			//	m_CurrentPlayState = PLAYING;

			//}else 
			{
				GstStateChangeReturn gscr = gst_element_set_state(m_GstPipeline, GST_STATE_PLAYING);

				//This is updated when server initiates a change to base time

				seekFrame(m_SeekTime);
				uint64_t tmp = m_SeekTime / (uint64_t)(1000000000);
				std::cout << " Client resuming playback from: " << tmp << "  seconds" << std::endl;

				m_CurrentPlayState = PLAYING;
			}
		}
	}
}

void GStreamerWrapper::stop(){
	if ( m_GstPipeline ){
		// Stop in this context now means a full clearing of the buffers in gstreamer
		gst_element_set_state( m_GstPipeline, GST_STATE_NULL );

		m_CurrentPlayState = STOPPED;
	}
}

void GStreamerWrapper::print_status_of_all()
{
	auto it = gst_bin_iterate_elements(GST_BIN(m_GstPipeline));
	GValue value = G_VALUE_INIT;

	for (GstIteratorResult r = gst_iterator_next(it, &value); r != GST_ITERATOR_DONE; r = gst_iterator_next(it, &value))
	{
		if (r == GST_ITERATOR_OK)
		{
			GstElement *e = static_cast<GstElement*>(g_value_peek_pointer(&value));
			GstState  current, pending;
			auto ret = gst_element_get_state(e, &current, &pending, 100000);
			std::cout << G_VALUE_TYPE_NAME(&value) << "(" << gst_element_get_name(e) << "), status = " << gst_element_state_get_name(current) << ", pending = " << gst_element_state_get_name(pending) << std::endl;
		}
	}
}
uint64_t GStreamerWrapper::getRunningTime(){
	uint64_t now = getPipelineTime();
	return now - gst_element_get_base_time(m_GstPipeline);
}

void GStreamerWrapper::pause(){

	if ( m_GstPipeline ){

		if (mServer) {

			m_SeekTime = getCurrentTimeInNs();
			uint64_t tmp = m_SeekTime / (uint64_t)(1000000000);
			std::cout << " pausing at: " << tmp << "  seconds" << std::endl;
		}


		GstStateChangeReturn gscr = gst_element_set_state(m_GstPipeline, GST_STATE_PAUSED);

		std::cout << "----------- Paused" << std::endl;


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
	std::cout << "Setting time position in NS: " << iTargetTimeInNs << std::endl;
	seekFrame( m_iCurrentTimeInNs );
}

void GStreamerWrapper::scrubToPosition( double fPos, float speed){
	if (fPos < 0.0)
		fPos = 0.0;
	else if (fPos > 1.0)
		fPos = 1.0;
	gint64 prevTime = m_iCurrentTimeInNs;

	m_dCurrentTimeInMs = fPos * m_dCurrentTimeInMs;
	m_iCurrentFrameNumber = (gint64)(fPos * m_iNumberOfFrames);
	m_iCurrentTimeInNs = (gint64)(fPos * m_iDurationInNs);


	GstFormat gstFormat = GST_FORMAT_TIME;

	// The flags determine how the seek behaves, in this case we simply want to jump to certain part in stream
	// while keeping the pre-set speed and play direction
	GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH || GST_SEEK_FLAG_TRICKMODE);


	gboolean bIsSeekSuccessful = false;

#if 1
	if (mServer) {
		uint64_t baseTime = getPipelineTime();
		setPipelineBaseTime(baseTime);
	}
#endif
	if (m_PlayDirection == FORWARD){
		bIsSeekSuccessful = gst_element_seek(GST_ELEMENT(m_GstPipeline),
			speed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			prevTime,
			GST_SEEK_TYPE_SET,
			m_iCurrentTimeInNs);
	}
#if 0
	else if (m_PlayDirection == BACKWARD)	{
		bIsSeekSuccessful = gst_element_seek(GST_ELEMENT(m_GstPipeline),
			-m_fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			0,
			GST_SEEK_TYPE_SET,
			iTargetTimeInNs);
	}
#endif
	if (!(bIsSeekSuccessful == 0)){
		m_PendingSeek = false;
	}

	//return bIsSeekSuccessful != 0;
//}

}

void GStreamerWrapper::setPosition(double fPos){
	if( fPos < 0.0 )
		fPos = 0.0;
	else if( fPos > 1.0 )
		fPos = 1.0;


		m_dCurrentTimeInMs = fPos * m_dCurrentTimeInMs;
		m_iCurrentFrameNumber = (gint64)(fPos * m_iNumberOfFrames);
		m_iCurrentTimeInNs = (gint64)(fPos * m_iDurationInNs);

		std::cout << "setting frame position: " << m_iCurrentTimeInNs << std::endl;
		seekFrame(m_iCurrentTimeInNs);

}

void GStreamerWrapper::setFastPosition(double fPos){
	if (fPos < 0.0)
		fPos = 0.0;
	else if (fPos > 1.0)
		fPos = 1.0;

	m_dCurrentTimeInMs = fPos * m_dCurrentTimeInMs;
	m_iCurrentFrameNumber = (gint64)(fPos * m_iNumberOfFrames);
	m_iCurrentTimeInNs = (gint64)(fPos * m_iDurationInNs);

	seekFast(m_iCurrentTimeInNs);
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

gint64					GStreamerWrapper::getBaseTime(){
	return m_BaseTime;
}


void					GStreamerWrapper::setSeekTime(uint64_t seek_time){
	m_SeekTime = seek_time;
	std::cout << "------ Changing seek time: " << m_SeekTime << std::endl;

}


gint64					GStreamerWrapper::getSeekTime(){
	return m_SeekTime;
}



gint64					GStreamerWrapper::getStartTime(){
	return m_StartTime;
}

void					GStreamerWrapper::setStartTime(uint64_t start_time){
	m_StartTime = start_time;
}
bool GStreamerWrapper::isFastSeeking(){
	return m_isFastSeeking;
}

bool GStreamerWrapper::seekFast(gint64 iTargetTimeInNs){
	m_isFastSeeking = true;
	GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_TRICKMODE_KEY_UNITS |  GST_SEEK_FLAG_TRICKMODE_NO_AUDIO);
	//GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_TRICKMODE_KEY_UNITS);

	gboolean bIsSeekSuccessful = false;
	GstFormat gstFormat = GST_FORMAT_TIME;

	std::cout << "++++++++++++++ Fast Seeking" << std::endl;

	if (mServer) {
		uint64_t baseTime = getPipelineTime();
		setPipelineBaseTime(baseTime);
		m_SeekTime = iTargetTimeInNs;
	}

		bIsSeekSuccessful = gst_element_seek(GST_ELEMENT(m_GstPipeline),
			m_fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			iTargetTimeInNs,
			GST_SEEK_TYPE_NONE,
			GST_CLOCK_TIME_NONE);

		return bIsSeekSuccessful!=0;
}

bool GStreamerWrapper::resetSeekMode(GstSeekFlags flags){
	GstEvent *seek_event;
	GstElement *video_sink;;

	m_isFastSeeking = false; 

	g_object_get(m_GstPipeline, "video-sink", &video_sink, NULL);
	std::cout << "++++++++++++++ Reset Seeking" << std::endl;
	

	// This intermediate event is needed to transition from seeking with SeekFast().   SeekFast uses a flag that greatly improves scrubbing performance 
	//(GST_SEEK_FLAG_TRICKMODE_KEY_UNITS).    For some reason, it takes several seconds for the pipeline to flush if a normal seek event/command is called with
	//only the GST_SEEK_FLAG_FLUSH flag.   However, calling a seek event that does no advancement and uses the GST_SEEK_FLAG_TRICKMODE causes the pipelie to flush/change immediately.
	// With this, the fPS count is reduced for a second or so.   Without this, the video updates about every 3-4 seconds, until the pipeline flush.
	seek_event = gst_event_new_seek(m_fSpeed, GST_FORMAT_TIME, flags,
		GST_SEEK_TYPE_NONE, 0, GST_SEEK_TYPE_NONE, 0);

	bool bIsSeekSuccessful = gst_element_send_event(video_sink, seek_event);
	return bIsSeekSuccessful != 0;
}

bool GStreamerWrapper::seekFrame( gint64 iTargetTimeInNs ){

	if (m_iDurationInNs < 0) {
		std::cout << "Not ready to seek.  Setting pending seek" << std::endl;
		m_PendingSeekTime = iTargetTimeInNs;
		m_PendingSeek = true;
		return false;
	}

	GstFormat gstFormat = GST_FORMAT_TIME;

	// The flags determine how the seek behaves, in this case we simply want to jump to certain part in stream
	// while keeping the pre-set speed and play direction
#if 1
	//GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_TRICKMODE);
	GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH  );
#else
	GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_TRICKMODE_KEY_UNITS | GST_SEEK_FLAG_TRICKMODE_NO_AUDIO);
#endif
	std::cout << "++++++++++++++ Normal Seeking" << std::endl;

	gboolean bIsSeekSuccessful = false;

#if 1
	if (mServer) {
		uint64_t baseTime = getPipelineTime();
		setPipelineBaseTime(baseTime);
#if 1
		m_SeekTime = iTargetTimeInNs;
#endif
	}
#endif
	std::cout << "base time: " << m_BaseTime << "   Seeking to : " << iTargetTimeInNs << std::endl;

	if (m_PlayDirection == FORWARD){
#if 1
		bIsSeekSuccessful = gst_element_seek( GST_ELEMENT( m_GstPipeline ),
			m_fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			iTargetTimeInNs,
			GST_SEEK_TYPE_NONE,
			GST_CLOCK_TIME_NONE );
#else

		GstEvent *seek_event;
		GstElement *video_sink;;
		g_object_get(m_GstPipeline, "video-sink", &video_sink, NULL);
		GstSeekFlags gstSeekFlags = (GstSeekFlags)(GST_SEEK_FLAG_FLUSH );

		seek_event = gst_event_new_seek(m_fSpeed, GST_FORMAT_TIME, gstSeekFlags,
			GST_SEEK_TYPE_SET, iTargetTimeInNs, GST_SEEK_TYPE_NONE, 0);
		/* Send the event */

		bIsSeekSuccessful = gst_element_send_event(video_sink, seek_event);

#if 0

		// This intermediate event is needed to transition from seeking with SeekFast().   SeekFast uses a flag that greatly improves scrubbing performance 
		//(GST_SEEK_FLAG_TRICKMODE_KEY_UNITS).    For some reason, it takes several seconds for the pipeline to flush if a normal seek event/command is called with
		//only the GST_SEEK_FLAG_FLUSH flag.   However, calling a seek event that does no advancement and uses the GST_SEEK_FLAG_TRICKMODE causes the pipelie to flush/change immediately.
		// With this, the fPS count is reduced for a second or so.   Without this, the video updates about every 3-4 seconds, until the pipeline flush.
		seek_event = gst_event_new_seek(m_fSpeed, GST_FORMAT_TIME, gstSeekFlags,
			GST_SEEK_TYPE_NONE, 0, GST_SEEK_TYPE_NONE, 0);
		
		bIsSeekSuccessful = gst_element_send_event(video_sink, seek_event);
		Sleep(1);

#else
//		bool tmpA = gst_element_send_event(video_sink, gst_event_new_step(GST_FORMAT_BUFFERS, 1, 1.0f, TRUE, FALSE) );

#endif



		
#endif
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

	if(m_VerboseLogging){
		DS_LOG_INFO("Got video info, duration=" << m_iDurationInNs << " Number of video streams: " << m_iNumVideoStreams << " audio: " << m_iNumAudioStreams);
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

						//if( (m_CurrentGstState == STATE_PLAYING || m_CurrentGstState == STATE_PAUSED) && m_PendingSeek){
						//	seekFrame(m_PendingSeekTime);
						//}

					  }

				break;

				case GST_MESSAGE_ASYNC_DONE :{
					//m_SeekTime = 0;
					std::cout << "Retrieving video info" << std::endl;
					retrieveVideoInfo();
					if ((m_CurrentGstState == STATE_PLAYING || m_CurrentGstState == STATE_PAUSED) && m_PendingSeek){
						std::cout << "servicing pending seek" << std::endl;
						seekFrame(m_PendingSeekTime);
					}

				}
				break;

				case GST_MESSAGE_NEW_CLOCK:{
					if(m_VerboseLogging){
						DS_LOG_INFO("Gst New clock");
					}
					// For example on net sync: http://noraisin.net/diary/?p=954
					// also: #include "gst/net/gstnettimeprovider.h"

					//GstClock* clock = gst_net_client_clock_new("new", "127.0.0.1", 6767, 0);
					//gst_pipeline_use_clock(GST_PIPELINE(m_GstPipeline), clock);
				}
				break;

				case GST_MESSAGE_SEGMENT_DONE : 
					{
					//m_SeekTime = 0;

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
									 if (mServer){
										 //Update the base time with the value of the pipeline/net clock.

										 //GstClock* clock_ = gst_pipeline_get_clock(GST_PIPELINE(m_GstPipeline));
										 /* m_BaseTime = getNetworkTime();
										 gst_element_set_base_time(m_GstPipeline, m_BaseTime);*/
										 setSeekTime(0);
										 m_playFromPause = true;
										 setPipelineBaseTime(getNetworkTime());
										 std::cout << "-----------  BASE TIME: " << m_BaseTime << std::endl;
									 }
									 else {
										 // We could have the client do a similar thing as the server, but instead we
										 // update the base time, and synchronize this base time with the client.
									 }
									 if (gst_element_seek(GST_ELEMENT(m_GstPipeline),
										 m_fSpeed,
										 GST_FORMAT_TIME,
										 (GstSeekFlags)(GST_SEEK_FLAG_FLUSH),// | GST_SEEK_FLAG_SEGMENT),
										 GST_SEEK_TYPE_SET,
										 0,
										 GST_SEEK_TYPE_SET,
										 m_iDurationInNs)){
										 play();

									 }
									 else{
										 DS_LOG_WARNING("Could not instantiate the GST client network clock.");
									 }

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

				case GST_MESSAGE_TAG :

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

void GStreamerWrapper::fastSeek(float speed) {
	m_fSpeed = speed;
	m_SeekTime += getRunningTime();
	setPipelineBaseTime(getPipelineTime());
	uint64_t twoSecs = 2000000000;

	gst_element_seek(m_GstPipeline,
		speed,
		GST_FORMAT_TIME,
		GST_SEEK_FLAG_SKIP,
		GST_SEEK_TYPE_SET, m_SeekTime,
		//GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
		GST_SEEK_TYPE_SET, m_SeekTime+ twoSecs);


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
	GstSample* gstVideoSinkBuffer = gst_app_sink_pull_sample( GST_APP_SINK( appsink ) );	
	( ( GStreamerWrapper * )listener )->newVideoSinkBufferCallback( gstVideoSinkBuffer );
	gst_sample_unref( gstVideoSinkBuffer );
	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewBufferFromAudioSource( GstAppSink* appsink, void* listener ){

	GstSample* gstAudioSinkBuffer = gst_app_sink_pull_sample( GST_APP_SINK( appsink ) );
	( ( GStreamerWrapper * )listener )->newAudioSinkBufferCallback( gstAudioSinkBuffer );
	gst_sample_unref( gstAudioSinkBuffer );

	return GST_FLOW_OK;
}

void GStreamerWrapper::newVideoSinkPrerollCallback(GstSample* videoSinkSample){
	if(!m_cVideoBuffer) return;

	std::lock_guard<decltype(m_VideoLock)> lock(m_VideoLock);
	GstBuffer* buff = gst_sample_get_buffer(videoSinkSample);	


	GstMapInfo map;
	GstMapFlags flags = GST_MAP_READ;
	gst_buffer_map(buff, &map, flags);

	unsigned int videoBufferSize = map.size; 

	// sanity check on buffer size, in case something weird happened.
	// In practice, this can fuck up the look of the video, but it plays and doesn't crash
	if(m_cVideoBufferSize != videoBufferSize){
		delete[] m_cVideoBuffer;
		m_cVideoBufferSize = videoBufferSize;
		m_cVideoBuffer = new unsigned char[m_cVideoBufferSize];
	} 

	memcpy((unsigned char *)m_cVideoBuffer, map.data, videoBufferSize);
	if(!m_PendingSeek) m_bIsNewVideoFrame = true;
	

	gst_buffer_unmap(buff, &map);
}

void GStreamerWrapper::newVideoSinkBufferCallback( GstSample* videoSinkSample ){
	if(!m_cVideoBuffer) return;

	std::lock_guard<decltype(m_VideoLock)> lock(m_VideoLock);
	if(!m_PendingSeek) m_bIsNewVideoFrame = true;

	GstBuffer* buff = gst_sample_get_buffer(videoSinkSample);	
	GstMapInfo map;
	GstMapFlags flags = GST_MAP_READ;
	gst_buffer_map(buff, &map, flags);


	unsigned int videoBufferSize = map.size;

	if(m_cVideoBufferSize != videoBufferSize){
		delete[] m_cVideoBuffer;
		m_cVideoBufferSize = videoBufferSize;
		m_cVideoBuffer = new unsigned char[m_cVideoBufferSize];
	}

	memcpy((unsigned char *)m_cVideoBuffer, map.data, videoBufferSize);
	if(!m_PendingSeek) m_bIsNewVideoFrame = true;

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

};
