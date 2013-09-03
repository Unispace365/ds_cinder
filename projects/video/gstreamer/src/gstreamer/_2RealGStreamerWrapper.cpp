/*
	CADET - Center for Advances in Digital Entertainment Technologies
	Copyright 2012 University of Applied Science Salzburg / MultiMediaTechnology

	http://www.cadet.at
	http://multimediatechnology.at/

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

	CADET - Center for Advances in Digital Entertainment Technologies

	Authors: Steven Stojanovic, Robert Praxmarer
	Web: http://www.1n0ut.com
	Email: stevesparrow07@googlemail.com, support@cadet.at
	Created: 07-09-2011

	This wrapper uses GStreamer, and is credited as follows:
*/
/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *
 * gst.h: Main header for GStreamer, apps should include this
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef LINUX
    #include <math.h>
    #include <cstring>
    #include "_2RealGStreamerWrapper.h"
#else
    #include "_2RealGStreamerWrapper.h"
#endif

#include <ds/debug/logger.h>
#include <iostream>

namespace _2RealGStreamerWrapper
{

/* this code is for making use of threads for the message handling, so you don't have to call update manually
   but this introduces a dependency to boost::thread so it's up to you what you prefer, it shouldn't make much
   difference in practical terms */
#ifdef THREADED_MESSAGE_HANDLER
gboolean onHandleGstMessages(GstBus *bus, GstMessage *msg, gpointer data)
{
     GStreamerWrapper* obj = (GStreamerWrapper*)(data);
     switch (GST_MESSAGE_TYPE(msg))
     {
		 case GST_MESSAGE_ERROR:
					GError *err;
					gchar *debug;
					gst_message_parse_error( msg, &err, &debug );
					std::cout << "Embedded video playback halted: module " << gst_element_get_name( GST_MESSAGE_SRC( msg ) ) << " reported " << err->message << std::endl;
					obj->close();
					g_error_free(err);
					g_free(debug);
			break;

        case GST_MESSAGE_EOS:
				switch ( obj->getLoopMode() )
					{
						case NO_LOOP:
							 obj->stop();
							break;

						case LOOP:
							obj->stop();
							obj->play();
							break;

						case BIDIRECTIONAL_LOOP:
							 obj->setDirection((PlayDirection)-obj->getDirection());
							 obj->stop();
							 obj->play();
							break;

						default:
							break;
					}

            break;
        default:
            break;
    }
    return TRUE;
}

void threadedMessageHandler(GStreamerWrapper* obj)
{
	obj->m_GMainLoop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (obj->m_GMainLoop);		//g_main loop is needed to catch user defined callbacks for detection of EOS, the GST EOS callback works, but apparently you can't manipulate the pipeline (seek) from in there
}
#endif


/******************************************************************************/

GStreamerWrapper::GStreamerWrapper() :
	m_bFileIsOpen( false ),
	m_cVideoBuffer( NULL ),
	m_cAudioBuffer( NULL ),
	m_GstPipeline( NULL ),
	m_GstVideoSink( NULL ),
	m_GstAudioSink( NULL ),
	m_GstBus( NULL )
{
	gst_init( NULL, NULL );

	m_CurrentPlayState = NOT_INITIALIZED;
}

GStreamerWrapper::GStreamerWrapper( std::string strFilename, bool bGenerateVideoBuffer, bool bGenerateAudioBuffer ) :
	m_bFileIsOpen( false ),
	m_cVideoBuffer( NULL ),
	m_cAudioBuffer( NULL ),
	m_GstPipeline( NULL ),
	m_GstVideoSink( NULL ),
	m_GstAudioSink( NULL ),
	m_GstBus( NULL )
{
	gst_init( NULL, NULL );

	open( strFilename, bGenerateVideoBuffer, bGenerateAudioBuffer );
}


GStreamerWrapper::~GStreamerWrapper()
{
	close();
}

bool GStreamerWrapper::open( std::string strFilename, bool bGenerateVideoBuffer, bool bGenerateAudioBuffer, bool isTransparent, int videoWidth, int videoHeight, double duration_inMS)
{
	// init property variables
	m_iNumVideoStreams = 0;
	m_iNumAudioStreams = 0;
	m_iCurrentVideoStream = 0;
	m_iCurrentAudioStream = 0;
	m_iWidth = m_iHeight = 0;
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
	m_strFilename = strFilename;


	if( m_bFileIsOpen )
	{
		stop();
		close();
	}

#ifdef THREADED_MESSAGE_HANDLER
		m_MsgHandlingThread = boost::thread( boost::bind(threadedMessageHandler, this));
#endif


	bool needsVideoInfo(true);
	if(videoWidth > 0 && videoHeight > 0 && duration_inMS > 0){
		needsVideoInfo = false;
		m_iWidth = videoWidth;
		m_iHeight = videoHeight;
		m_dDurationInMs = duration_inMS;
		m_iDurationInNs = (gint64)duration_inMS * 1000000;
	}

	////////////////////////////////////////////////////////////////////////// PIPELINE
	// Init main pipeline --> playbin2
	m_GstPipeline = gst_element_factory_make( "playbin2", "pipeline" );

	// Check and re-arrange filename string
	if ( strFilename.find( "file:/", 0 ) == std::string::npos &&
		 strFilename.find( "file:///", 0 ) == std::string::npos &&
		 strFilename.find( "http://", 0 ) == std::string::npos )
	{
		strFilename = "file:/" + strFilename;
	}

	// Open Uri
	g_object_set( m_GstPipeline, "uri", strFilename.c_str(), NULL );

	////////////////////////////////////////////////////////////////////////// VIDEO SINK
	// Extract and Config Video Sink
	if ( bGenerateVideoBuffer )
	{
		// Create the video appsink and configure it
		m_GstVideoSink = gst_element_factory_make( "appsink", "videosink" );
		gst_base_sink_set_sync( GST_BASE_SINK( m_GstVideoSink ), true );
		gst_app_sink_set_max_buffers( GST_APP_SINK( m_GstVideoSink ), 8 );
		gst_app_sink_set_drop( GST_APP_SINK( m_GstVideoSink ),true );
		gst_base_sink_set_max_lateness( GST_BASE_SINK( m_GstVideoSink ), -1);

		// Set some fix caps for the video sink
		// It would seem that GStreamer then tries to transform any incoming video stream according to these caps
		GstCaps* caps;
		if(isTransparent){
			caps = gst_caps_new_simple( "video/x-raw-rgb",
			"bpp", G_TYPE_INT, 32,
			//"depth", G_TYPE_INT, 24,
			"endianness",G_TYPE_INT,4321,
			"red_mask",G_TYPE_INT,0xff000000,
			"green_mask",G_TYPE_INT,0x00ff0000,
			"blue_mask",G_TYPE_INT,0x0000ff00,
			"alpha_mask",G_TYPE_INT,0x000000ff,
			NULL );
		} else if(videoWidth > 0) {
			if(videoWidth % 4 != 0){
				videoWidth += 4 - videoWidth % 4;
			}
			caps= gst_caps_new_simple( "video/x-raw-rgb",
				"bpp", G_TYPE_INT, 24,
				"depth", G_TYPE_INT, 24,
				"width", G_TYPE_INT, videoWidth,
				"endianness",G_TYPE_INT,4321,
				"red_mask",G_TYPE_INT,0xff0000,
				"green_mask",G_TYPE_INT,0x00ff00,
				"blue_mask",G_TYPE_INT,0x0000ff,
				NULL );
		} else {
			caps= gst_caps_new_simple( "video/x-raw-rgb",
				"bpp", G_TYPE_INT, 24,
				"depth", G_TYPE_INT, 24,
				"endianness",G_TYPE_INT,4321,
				"red_mask",G_TYPE_INT,0xff0000,
				"green_mask",G_TYPE_INT,0x00ff00,
				"blue_mask",G_TYPE_INT,0x0000ff,
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
		m_GstVideoSinkCallbacks.new_buffer = &GStreamerWrapper::onNewBufferFromVideoSource;
		gst_app_sink_set_callbacks( GST_APP_SINK( m_GstVideoSink ), &m_GstVideoSinkCallbacks, this, NULL );
	}
	else
	{
#ifdef _WIN32 // Use direct show as playback plugin if on Windows; Needed for features like play direction and playback speed to work correctly
		GstElement* videoSink = gst_element_factory_make( "directdrawsink", NULL );
		g_object_set( m_GstPipeline, "video-sink", videoSink, NULL );
#elif LINUX
        GstElement* videoSink = gst_element_factory_make( "xvimagesink", NULL );    //possible alternatives: ximagesink (no (gpu) fancy stuff) or better: cluttersink
		g_object_set( m_GstPipeline, "video-sink", videoSink, NULL );
#else // Use Mac OSX plugin otherwise
		GstElement* videoSink = gst_element_factory_make( "osxvideosink", NULL );
		g_object_set( m_GstPipeline, "video-sink", videoSink, NULL );
#endif
	}

	////////////////////////////////////////////////////////////////////////// AUDIO SINK
	// Extract and config Audio Sink
	if ( bGenerateAudioBuffer )
	{
		// Create and configure audio appsink
		m_GstAudioSink = gst_element_factory_make( "appsink", "audiosink" );
		gst_base_sink_set_sync( GST_BASE_SINK( m_GstAudioSink ), true );
		// Set the configured audio appsink to the main pipeline
		g_object_set( m_GstPipeline, "audio-sink", m_GstAudioSink, (void*)NULL );
		// Tell the video appsink that it should not emit signals as the buffer retrieving is handled via callback methods
		g_object_set( m_GstAudioSink, "emit-signals", false, "sync", true, (void*)NULL );

		// Set Audio Sink callback methods
		m_GstAudioSinkCallbacks.eos = &GStreamerWrapper::onEosFromAudioSource;
		m_GstAudioSinkCallbacks.new_preroll = &GStreamerWrapper::onNewPrerollFromAudioSource;
		m_GstAudioSinkCallbacks.new_buffer = &GStreamerWrapper::onNewBufferFromAudioSource;
		gst_app_sink_set_callbacks( GST_APP_SINK( m_GstAudioSink ), &m_GstAudioSinkCallbacks, this, NULL );
	}
	else
	{
#ifdef _WIN32 // Use direct sound plugin if on Windows; Needed for features like play direction and playback speed to work correctly
		GstElement* audioSink = gst_element_factory_make( "directsoundsink", NULL );
		if (!audioSink) { DS_LOG_WARNING("GStreamer can't create audio sink"); }
		g_object_set ( m_GstPipeline, "audio-sink", audioSink, NULL );
#elif LINUX
		GstElement* audioSink = gst_element_factory_make( "pulsesink", NULL );  //alternative: alsasink
		g_object_set ( m_GstPipeline, "audio-sink", audioSink, NULL );
#else // Use Mac OSC plugin otherwise
		GstElement* audioSink = gst_element_factory_make( "osxaudiosink", NULL );
		g_object_set ( m_GstPipeline,"audio-sink", audioSink, NULL );
#endif
	}

	////////////////////////////////////////////////////////////////////////// BUS
	// Set GstBus
	m_GstBus = gst_pipeline_get_bus( GST_PIPELINE( m_GstPipeline ) );

	if ( m_GstPipeline != NULL )
	{
//just add this callback for threaded message handling
#ifdef THREADED_MESSAGE_HANDLER
		gst_bus_add_watch (m_GstBus, onHandleGstMessages, this );
#endif
		// We need to stream the file a little bit in order to be able to retrieve information from it
		gst_element_set_state( m_GstPipeline, GST_STATE_READY );
		gst_element_set_state( m_GstPipeline, GST_STATE_PAUSED );

		// For some reason this is needed in order to gather video information such as size, framerate etc ...
		if(needsVideoInfo){
			GstState state;
			gst_element_get_state( m_GstPipeline, &state, NULL, 20 * GST_SECOND );
			// Retrieve and store all relevant Media Information
			if(!retrieveVideoInfo()){
				return false;
			}
		}
		m_CurrentPlayState = OPENED;
	}


	//if( !hasVideo() && !hasAudio() )	// is a valid multimedia file?
	//{
	//	DS_LOG_WARNING("Couldn't detect any audio or video streams in this file! " << strFilename);
	//	//close();
	//	return false;
	//}

	// Print Media Info
//	printMediaFileInfo();

	// TODO: Check if everything was initialized correctly
	// A file has been opened
	m_bFileIsOpen = true;

	return true;
}

void GStreamerWrapper::close()
{
	m_bFileIsOpen = false;
	m_CurrentPlayState = NOT_INITIALIZED;


// get rid of message handler thread
#ifdef THREADED_MESSAGE_HANDLER
	g_main_loop_quit(m_GMainLoop);
	m_MsgHandlingThread.join();
#endif

	if ( m_GstPipeline != NULL )
	{
		gst_element_set_state( m_GstPipeline, GST_STATE_NULL );
		gst_object_unref( m_GstPipeline );
		m_GstPipeline = NULL;
		m_GstVideoSink = NULL;
		m_GstAudioSink = NULL;
	}

	// unreffing the pipeline should free the sink
	if ( m_GstVideoSink != NULL )
	{
		gst_object_unref( m_GstVideoSink );
		m_GstVideoSink = NULL;
	}

	// unreffing the pipeline should free the sink
	if ( m_GstAudioSink != NULL )
	{
		gst_object_unref( m_GstAudioSink );
		m_GstAudioSink = NULL;
	}

	if ( m_GstBus != NULL )
	{
		gst_object_unref( m_GstBus );
		m_GstBus = NULL;
	}

	delete [] m_cVideoBuffer;
	m_cVideoBuffer = NULL;

	delete [] m_cAudioBuffer;
	m_cAudioBuffer = NULL;
}

void GStreamerWrapper::update()
{
#ifndef THREADED_MESSAGE_HANDLER
	handleGStMessage();
#endif
}

void GStreamerWrapper::play()
{
	if ( m_GstPipeline != NULL )
	{
		GstStateChangeReturn gscr = gst_element_set_state( m_GstPipeline, GST_STATE_PLAYING );
		if (gscr == GST_STATE_CHANGE_FAILURE) {
			DS_LOG_WARNING("Error going to playing state (error=" << gscr << ")");
		}
		m_CurrentPlayState = PLAYING;
	}
}

void GStreamerWrapper::stop()
{
	if ( m_GstPipeline != NULL )
	{
		// "Hack" for stopping ...
		//gst_element_set_state( m_GstPipeline, GST_STATE_PAUSED );
		// The original setting of state paused was hacky, they were using the stop
		// call in the looping mechanism. Why not just seek there? 
		// Stop in this context now means a full clearing of the buffers in gstreamer
		gst_element_set_state( m_GstPipeline, GST_STATE_NULL );

		//if ( m_PlayDirection == FORWARD )
		//	seekFrame( 0 );
		//else if ( m_PlayDirection == BACKWARD )
		//	seekFrame( m_iDurationInNs );

		m_CurrentPlayState = STOPPED;
	}
}

void GStreamerWrapper::pause()
{
	if ( m_GstPipeline != NULL )
	{
		gst_element_set_state( m_GstPipeline, GST_STATE_PAUSED );
		m_CurrentPlayState = PAUSED;
	}
}

void GStreamerWrapper::printMediaFileInfo()
{
//	return;

	std::cout << "-----------------------------------------------------------------" << std::endl;
	std::cout << "Loading file ..." << std::endl;
	std::cout << "> File Uri: " << m_strFilename << std::endl;
	std::cout << "> Duration in NanoSeconds: " << m_iDurationInNs << std::endl;
	std::cout << "> Video Streams: " << m_iNumVideoStreams << std::endl;
	std::cout << "> Audio Streams: " << m_iNumAudioStreams << std::endl;

	if ( m_iNumVideoStreams > 0 )
	{
		std::cout << std::endl << "Video Information ..." << std::endl;
		std::cout << "> Number of Frames: " << m_iNumberOfFrames << std::endl;
		std::cout << "> Video Width: " << m_iWidth << std::endl;
		std::cout << "> Video Height: " << m_iHeight << std::endl;
		std::cout << "> FPS: " << m_fFps << std::endl;
	}

	if ( m_iNumAudioStreams > 0 )
	{
		std::cout << std::endl << "Audio Information ..." << std::endl;
		std::cout << "> Sample Rate: " << m_iAudioSampleRate << std::endl;
		std::cout << "> Channels: " << m_iNumAudioChannels << std::endl;
		std::cout << "> Audio Buffer Size: " << m_iAudioBufferSize << std::endl;
		std::cout << "> Audio Decode Buffer Size: " << m_iAudioDecodeBufferSize << std::endl;
		std::cout << "> Is Audio Signed: " << m_bIsAudioSigned << std::endl;
		std::cout << "> Audio Width: " << m_iAudioWidth << std::endl;
		std::cout << "> Audio Endianness: " << m_AudioEndianness << std::endl;
	}
	std::cout << "-----------------------------------------------------------------" << std::endl;
}

void GStreamerWrapper::setCurrentVideoStream( int iCurrentVideoStream )
{
	if ( m_iCurrentVideoStream != iCurrentVideoStream )
	{
		if ( iCurrentVideoStream >= 0 && iCurrentVideoStream < m_iNumVideoStreams )
		{
			m_iCurrentVideoStream = iCurrentVideoStream;

			g_object_set( m_GstPipeline, "current-video", m_iCurrentVideoStream, NULL );
		}
	}
}

void GStreamerWrapper::setCurrentAudioStream( int iCurrentAudioStream )
{
	if ( m_iCurrentAudioStream != iCurrentAudioStream )
	{
		if ( iCurrentAudioStream >= 0 && iCurrentAudioStream < m_iNumAudioStreams )
		{
			m_iCurrentAudioStream = iCurrentAudioStream;

			g_object_set( m_GstPipeline, "current-audio", m_iCurrentAudioStream, NULL );
		}
	}
}

void GStreamerWrapper::setSpeed( float fSpeed )
{
	if( fSpeed != m_fSpeed )
	{
		m_fSpeed = fSpeed;
		if ( m_fSpeed < 0.0f )
			m_fSpeed = 0.0f;

		changeSpeedAndDirection( m_fSpeed, m_PlayDirection );
	}
}

void GStreamerWrapper::setDirection( PlayDirection direction )
{
	if ( m_PlayDirection != direction )
	{
		m_PlayDirection = direction;
		changeSpeedAndDirection( m_fSpeed, m_PlayDirection );
	}
}

void GStreamerWrapper::setLoopMode( LoopMode loopMode )
{
	m_LoopMode = loopMode;
}

void GStreamerWrapper::setFramePosition( gint64 iTargetFrameNumber )
{
	m_iCurrentFrameNumber = iTargetFrameNumber;

	setPosition( (float)m_iCurrentFrameNumber / (float)m_iNumberOfFrames );
}

void GStreamerWrapper::setTimePositionInMs( double dTargetTimeInMs )
{
	m_dCurrentTimeInMs = dTargetTimeInMs;
	seekFrame( (gint64)(m_dCurrentTimeInMs * 1000000) );
}

void GStreamerWrapper::setTimePositionInNs( gint64 iTargetTimeInNs )
{
	m_iCurrentTimeInNs = iTargetTimeInNs;
	seekFrame( m_iCurrentTimeInNs );
}

void GStreamerWrapper::setPosition( float fPos )
{
	if( fPos < 0.0 )
		fPos = 0.0;
	else if( fPos > 1.0 )
		fPos = 1.0;

	// This isn't being used for anything, is probably wrong, and should probably be removed
	m_dCurrentTimeInMs = fPos * m_dCurrentTimeInMs;
	m_iCurrentFrameNumber = (gint64)(fPos * m_iNumberOfFrames);
	m_iCurrentTimeInNs = (gint64)(fPos * m_iDurationInNs);

	seekFrame( m_iCurrentTimeInNs );
}

//bool GStreamerWrapper::hasVideo()
//{
//	return m_ContentType == VIDEO_AND_AUDIO || m_ContentType == VIDEO;
//}
//
//bool GStreamerWrapper::hasAudio()
//{
//	return m_ContentType == VIDEO_AND_AUDIO || m_ContentType == AUDIO;
//}

std::string GStreamerWrapper::getFileName()
{
	return m_strFilename;
}

unsigned char* GStreamerWrapper::getVideo()
{
	m_bIsNewVideoFrame = false;
	return m_cVideoBuffer;
}

int GStreamerWrapper::getCurrentVideoStream()
{
	return m_iCurrentVideoStream;
}

int GStreamerWrapper::getCurrentAudioStream()
{
	return m_iCurrentAudioStream;
}

int GStreamerWrapper::getNumberOfVideoStreams()
{
	return m_iNumVideoStreams;
}

int GStreamerWrapper::getNumberOfAudioStreams()
{
	return m_iNumAudioStreams;
}

unsigned int GStreamerWrapper::getWidth()
{
	return m_iWidth;
}

unsigned int GStreamerWrapper::getHeight()
{
	return m_iHeight;
}

bool GStreamerWrapper::isNewVideoFrame()
{
	return m_bIsNewVideoFrame;
}

float GStreamerWrapper::getFps()
{
	DS_LOG_WARNING("FPS may not be correct!");
	return m_fFps;
}

float GStreamerWrapper::getSpeed()
{
	return m_fSpeed;
}

float GStreamerWrapper::getPosition()
{
	return (float)getCurrentTimeInNs() / (float)m_iDurationInNs;
}

gint64 GStreamerWrapper::getCurrentFrameNumber()
{
	m_iCurrentFrameNumber = (gint64)(floor( (double)getCurrentTimeInMs() / 1000.0 * m_fFps ));
	return m_iCurrentFrameNumber;
}

gint64 GStreamerWrapper::getNumberOfFrames()
{
	DS_LOG_WARNING("Number of frames may not be correct!");
	return m_iNumberOfFrames;
}

double GStreamerWrapper::getCurrentTimeInMs()
{
	return (double)(getCurrentTimeInNs() / 1000000);
}

double GStreamerWrapper::getDurationInMs()
{
	return m_dDurationInMs;
}

gint64 GStreamerWrapper::getCurrentTimeInNs()
{
	GstFormat gstFormat = GST_FORMAT_TIME;
	gst_element_query_position( GST_ELEMENT( m_GstPipeline ), &gstFormat, &m_iCurrentTimeInNs );
	return m_iCurrentTimeInNs;
}

gint64 GStreamerWrapper::getDurationInNs()
{
	return m_iDurationInNs;
}

PlayState GStreamerWrapper::getState()
{
	return m_CurrentPlayState;
}

PlayDirection GStreamerWrapper::getDirection()
{
	return m_PlayDirection;
}

LoopMode GStreamerWrapper::getLoopMode()
{
	return m_LoopMode;
}

//ContentType GStreamerWrapper::getContentType()
//{
//	return m_ContentType;
//}

void GStreamerWrapper::setVolume( float fVolume )
{
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

unsigned char* GStreamerWrapper::getAudio()
{
	return m_cAudioBuffer;
}

bool GStreamerWrapper::getIsAudioSigned()
{
	return m_bIsAudioSigned;
}

int	GStreamerWrapper::getNumOfAudioChannels()
{
	return m_iNumAudioChannels;
}

int GStreamerWrapper::getAudioSampleRate()
{
	return m_iAudioSampleRate;
}

int GStreamerWrapper::getAudioBufferSize()
{
	return m_iAudioBufferSize;
}

int GStreamerWrapper::getAudioDecodeBufferSize()
{
	return m_iAudioDecodeBufferSize;
}

int GStreamerWrapper::getAudioWidth()
{
	return m_iAudioWidth;
}

float GStreamerWrapper::getCurrentVolume()
{
	return m_fVolume;
}

Endianness GStreamerWrapper::getAudioEndianness()
{
	return m_AudioEndianness;
}

bool GStreamerWrapper::seekFrame( gint64 iTargetTimeInNs )
{

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

	if ( m_PlayDirection == FORWARD )
	{
		bIsSeekSuccessful = gst_element_seek( GST_ELEMENT( m_GstPipeline ),
			m_fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			iTargetTimeInNs,
			GST_SEEK_TYPE_NONE,
			-1 );
	}
	else if ( m_PlayDirection == BACKWARD )
	{
		bIsSeekSuccessful = gst_element_seek( GST_ELEMENT( m_GstPipeline ),
			-m_fSpeed,
			gstFormat,
			gstSeekFlags,
			GST_SEEK_TYPE_SET,
			0,
			GST_SEEK_TYPE_SET,
			iTargetTimeInNs );
	}

	//std::cout << "Seeking frame: " << iTargetTimeInNs << " " << (float)(iTargetTimeInNs) / (float)(m_iDurationInNs) << " " << bIsSeekSuccessful << std::endl;

	if(!(bIsSeekSuccessful == 0)){
		m_PendingSeek = false;
	}
	return !(bIsSeekSuccessful==0);
}

bool GStreamerWrapper::changeSpeedAndDirection( float fSpeed, PlayDirection direction )
{
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

	return !(bIsSeekSuccessful == 0);
}

bool GStreamerWrapper::retrieveVideoInfo()
{
	////////////////////////////////////////////////////////////////////////// Media Duration
	// Nanoseconds
	GstFormat gstFormat = GST_FORMAT_TIME;
	gst_element_query_duration( GST_ELEMENT( m_GstPipeline ), &gstFormat, &m_iDurationInNs );

	// Milliseconds
	m_dDurationInMs = (double)(GST_TIME_AS_MSECONDS( m_iDurationInNs ));

	////////////////////////////////////////////////////////////////////////// Stream Info
	// Number of Video Streams
	g_object_get( m_GstPipeline, "n-video", &m_iNumVideoStreams, NULL );

	// Number of Audio Streams
	g_object_get( m_GstPipeline, "n-audio", &m_iNumAudioStreams, NULL );

	// Set Content Type according to the number of available Video and Audio streams
	//if ( m_iNumVideoStreams > 0 && m_iNumAudioStreams > 0 )
	//	m_ContentType = VIDEO_AND_AUDIO;
	//else if ( m_iNumVideoStreams > 0 )
	//	m_ContentType = VIDEO;
	//else if ( m_iNumAudioStreams > 0 )
	//	m_ContentType = AUDIO;
	
	if( m_iNumAudioStreams < 1 && m_iNumVideoStreams < 1){
		DS_LOG_WARNING("No media streams detected in file.");
		return false;
	}

	////////////////////////////////////////////////////////////////////////// Video Data
	if ( m_iNumVideoStreams > 0 )
	{
		GstPad* gstPad = gst_element_get_static_pad( m_GstVideoSink, "sink" );
		if ( gstPad )
		{
			// Video Size
			gst_video_get_size( GST_PAD( gstPad ), &m_iWidth, &m_iHeight );

			// Frame Rate
			const GValue* framerate = gst_video_frame_rate( gstPad );

			int iFpsNumerator = gst_value_get_fraction_numerator( framerate );
			int iFpsDenominator = gst_value_get_fraction_denominator( framerate );

			// Number of frames
			m_iNumberOfFrames = (gint64)((float)( m_iDurationInNs / GST_SECOND ) * (float)iFpsNumerator / (float)iFpsDenominator);

			// FPS
			m_fFps = (float)iFpsNumerator / (float)iFpsDenominator;


			gst_object_unref( gstPad );
		}

		if(m_iWidth < 1 || m_iHeight < 1){
			DS_LOG_WARNING( "Error finding the size of the video!" );
			return false;
		}
	}

	return true;
}

void GStreamerWrapper::handleGStMessage()
{
	if ( m_GstBus != NULL )
	{
		while ( gst_bus_have_pending( m_GstBus ) )
		{
			m_GstMessage = gst_bus_pop( m_GstBus );

			if ( m_GstMessage != NULL )
			{
				// std::cout << "Message Type: " << GST_MESSAGE_TYPE_NAME( m_GstMessage ) << std::endl;

				GError* err;
				gchar* debug;

				switch ( GST_MESSAGE_TYPE( m_GstMessage ) )
				{
				case GST_MESSAGE_INFO:
					gst_message_parse_info(m_GstMessage, &err, &debug);
					DS_LOG_INFO("GST_MESSAGE_INFO module " << gst_element_get_name( GST_MESSAGE_SRC( m_GstMessage ) ) << " reported " << err->message);
					g_error_free(err);
					g_free(debug);
					break;

				case GST_MESSAGE_WARNING:
					gst_message_parse_warning(m_GstMessage, &err, &debug);
					DS_LOG_WARNING("GST_MESSAGE_WARNING module " << gst_element_get_name( GST_MESSAGE_SRC( m_GstMessage ) ) << " reported " << err->message);
					g_error_free(err);
					g_free(debug);
					break;
				case GST_MESSAGE_ERROR:
					gst_message_parse_error( m_GstMessage, &err, &debug );
					DS_LOG_WARNING("GST_MESSAGE_ERROR module " << gst_element_get_name( GST_MESSAGE_SRC( m_GstMessage ) ) << " reported " << err->message);
					std::cout << "Embedded video playback halted: module " << gst_element_get_name( GST_MESSAGE_SRC( m_GstMessage ) ) <<" reported " << err->message << std::endl;
					//close();
					//m_PendingClose = true;

					g_error_free(err);
					g_free(debug);
					break;

				case GST_MESSAGE_STATE_CHANGED:
					{
						GstState old, newState, pending;
						gst_message_parse_state_changed (m_GstMessage, &old, &newState, &pending);
						if (newState == GST_STATE_PLAYING) {
							m_CurrentGstState = STATE_PLAYING;
						} else if(newState == GST_STATE_NULL){
							m_CurrentGstState = STATE_NULL;
						} else if(newState == GST_STATE_PAUSED){
							m_CurrentGstState = STATE_PAUSED;
						} else if(newState == GST_STATE_READY){
							m_CurrentGstState = STATE_READY;
						}
//						std::cout << "\tstate=" << newState << " pending=" << pending << std::endl;
						if(m_PendingSeek && (m_CurrentGstState == STATE_PLAYING || m_CurrentGstState == STATE_PAUSED)){
							seekFrame(m_PendingSeekTime);
						}
					}
					break;
				case GST_MESSAGE_SEGMENT_DONE :{
					gst_element_seek( GST_ELEMENT( m_GstPipeline ),
						m_fSpeed,
						GST_FORMAT_TIME,
						GST_SEEK_FLAG_SEGMENT,
						GST_SEEK_TYPE_SET,
						0,
						GST_SEEK_TYPE_SET,
						m_iDurationInNs );
				}
											break;

				case GST_MESSAGE_ASYNC_DONE : {
					retrieveVideoInfo();
											  }
											  break;

				case GST_MESSAGE_EOS:
					switch ( m_LoopMode )
					{
						case NO_LOOP:
							pause();
							if(mVideoCompleteCallback) mVideoCompleteCallback(this);
							break;

						case LOOP:{
							gst_element_seek( GST_ELEMENT( m_GstPipeline ),
								m_fSpeed,
								GST_FORMAT_TIME,
								(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_SEGMENT),
								GST_SEEK_TYPE_SET,
								0,
								GST_SEEK_TYPE_SET,
								m_iDurationInNs );
							play();
							}
							break;

						case BIDIRECTIONAL_LOOP:
							std::cout << "bi-directional looping not implemented!" << std::endl;
							//m_PlayDirection = (PlayDirection)-m_PlayDirection;
							//stop();
							//play();
							break;

						default:
							break;
					}
					break;

				default:
					break;
				}
			}

			gst_message_unref( m_GstMessage );
		}
	}
}

void GStreamerWrapper::onEosFromVideoSource( GstAppSink* appsink, void* listener )
{
	( ( GStreamerWrapper *)listener )->videoEosCallback();
}

void GStreamerWrapper::onEosFromAudioSource( GstAppSink* appsink, void* listener )
{
	( ( GStreamerWrapper *)listener )->audioEosCallback();
}

GstFlowReturn GStreamerWrapper::onNewPrerollFromVideoSource( GstAppSink* appsink, void* listener )
{
	GstBuffer* gstVideoSinkBuffer = gst_app_sink_pull_preroll( GST_APP_SINK( appsink ) );
	( ( GStreamerWrapper *)listener )->newVideoSinkPrerollCallback( gstVideoSinkBuffer );
	gst_buffer_unref( gstVideoSinkBuffer );

	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewPrerollFromAudioSource( GstAppSink* appsink, void* listener )
{
	GstBuffer* gstAudioSinkBuffer = gst_app_sink_pull_preroll( GST_APP_SINK( appsink ) );
	( ( GStreamerWrapper * )listener )->newAudioSinkPrerollCallback( gstAudioSinkBuffer );
	gst_buffer_unref( gstAudioSinkBuffer );

	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewBufferFromVideoSource( GstAppSink* appsink, void* listener )
{
	GstBuffer* gstVideoSinkBuffer = gst_app_sink_pull_buffer( GST_APP_SINK( appsink ) );
	( ( GStreamerWrapper * )listener )->newVideoSinkBufferCallback( gstVideoSinkBuffer );
	gst_buffer_unref( gstVideoSinkBuffer );

	return GST_FLOW_OK;
}

GstFlowReturn GStreamerWrapper::onNewBufferFromAudioSource( GstAppSink* appsink, void* listener )
{
	GstBuffer* gstAudioSinkBuffer = gst_app_sink_pull_buffer( GST_APP_SINK( appsink ) );
	( ( GStreamerWrapper * )listener )->newAudioSinkBufferCallback( gstAudioSinkBuffer );
	gst_buffer_unref( gstAudioSinkBuffer );

	return GST_FLOW_OK;
}

void GStreamerWrapper::newVideoSinkPrerollCallback( GstBuffer* videoSinkBuffer )
{
	// Allocate memory for the video pixels according to the vide appsink buffer size
	if ( m_cVideoBuffer == NULL )
	{
		if(!m_PendingSeek) m_bIsNewVideoFrame = true;
		unsigned int videoBufferSize = (unsigned int)(GST_BUFFER_SIZE( videoSinkBuffer ));
		m_cVideoBuffer = new unsigned char[videoBufferSize];
	}

	// Copy the video appsink buffer data to our unsigned char array
	memcpy( (unsigned char *)m_cVideoBuffer, (unsigned char *)GST_BUFFER_DATA( videoSinkBuffer ), GST_BUFFER_SIZE( videoSinkBuffer ) );
	if(m_cVideoBuffer[0] == 255 && m_cVideoBuffer[1] == 255 && m_cVideoBuffer[2] == 255){
		DS_LOG_WARNING("White frame detected");
	}
}

void GStreamerWrapper::newVideoSinkBufferCallback( GstBuffer* videoSinkBuffer )
{
	if(!m_PendingSeek) m_bIsNewVideoFrame = true;

	// Copy the video appsink buffer data to our unsigned char array
	memcpy( (unsigned char *)m_cVideoBuffer, (unsigned char *)GST_BUFFER_DATA( videoSinkBuffer ), GST_BUFFER_SIZE( videoSinkBuffer ) );
	if(m_cVideoBuffer[0] == 255 && m_cVideoBuffer[1] == 255 && m_cVideoBuffer[2] == 255){
		DS_LOG_WARNING("White frame detected");
	}
}

void GStreamerWrapper::videoEosCallback()
{

}

void GStreamerWrapper::newAudioSinkPrerollCallback( GstBuffer* audioSinkBuffer )
{
	if ( m_cAudioBuffer == NULL )
	{
		m_iAudioBufferSize = GST_BUFFER_SIZE( audioSinkBuffer );
		m_cAudioBuffer = new unsigned char[m_iAudioBufferSize];

		////////////////////////////////////////////////////////////////////////// AUDIO DATA

		/*
			Note: For some reason, with this version of GStreamer the only way to retrieve the audio metadata
			is to read the caps from the audio appsink buffer and via a GstStructure we can retrieve the needed
			values from the caps. After lots of research I also found another possibility by using GstAudioInfo
			but this struct is not available in this version.

			If a later version of GStreamer is ever compiled in a valid way so it can be used with Visual Studio
			it would definitely be a good idea to retrieve the audio information somewhere else in the code.
			But this piece of code does it well for now.
		*/

		// Get Audio metadata
		// http://gstreamer.freedesktop.org/data/doc/gstreamer/head/pwg/html/section-types-definitions.html
		GstCaps* audioCaps = gst_buffer_get_caps( audioSinkBuffer );
		GstStructure* gstStructure = gst_caps_get_structure( audioCaps, 0 );

		// Is audio data signed or not?
		gboolean isAudioSigned;
		gst_structure_get_boolean( gstStructure, "signed", &isAudioSigned );
		m_bIsAudioSigned = !(isAudioSigned == 0);

		// Number of channels
		gst_structure_get_int( gstStructure, "channels", &m_iNumAudioChannels );
		// Audio sample rate
		gst_structure_get_int( gstStructure, "rate", &m_iAudioSampleRate );
		// Audio width
		gst_structure_get_int( gstStructure, "width", &m_iAudioWidth );

		// Calculate the audio buffer size without the number of channels and audio width
		m_iAudioDecodeBufferSize = m_iAudioBufferSize / m_iNumAudioChannels / ( m_iAudioWidth / 8 );

		// Audio endianness
		gint audioEndianness;
		gst_structure_get_int( gstStructure, "endianness",  &audioEndianness );
		m_AudioEndianness = (Endianness)audioEndianness;

		gst_caps_unref( audioCaps );
	}
	else
	{
		// The Audio Buffer size may change during runtime so we keep track if the buffer changes
		// If so, delete the old buffer and re-allocate it with the respective new buffer size
		int bufferSize = GST_BUFFER_SIZE( audioSinkBuffer );
		if ( m_iAudioBufferSize != bufferSize )
		{
			// Allocate the audio data array according to the audio appsink buffer size
			m_iAudioBufferSize = bufferSize;
			delete [] m_cAudioBuffer;
			m_cAudioBuffer = NULL;

			m_cAudioBuffer = new unsigned char[m_iAudioBufferSize];
		}
	}

	// Copy the audio appsink buffer data to our unsigned char array
	memcpy( (unsigned char *)m_cAudioBuffer, (unsigned char *)GST_BUFFER_DATA( audioSinkBuffer ), GST_BUFFER_SIZE( audioSinkBuffer ) );
}

void GStreamerWrapper::newAudioSinkBufferCallback( GstBuffer* audioSinkBuffer )
{
	// The Audio Buffer size may change during runtime so we keep track if the buffer changes
	// If so, delete the old buffer and re-allocate it with the respective new buffer size
	int bufferSize = GST_BUFFER_SIZE( audioSinkBuffer );

	if ( m_iAudioBufferSize != bufferSize )
	{
		m_iAudioBufferSize = bufferSize;
		delete [] m_cAudioBuffer;
		m_cAudioBuffer = NULL;

		m_cAudioBuffer = new unsigned char[m_iAudioBufferSize];

		// Recalculate the audio decode buffer size due to change in buffer size
		m_iAudioDecodeBufferSize = m_iAudioBufferSize / m_iNumAudioChannels / ( m_iAudioWidth / 8 );
	}

	// Copy the audio appsink buffer data to our unsigned char array
	memcpy( (unsigned char *)m_cAudioBuffer, (unsigned char *)GST_BUFFER_DATA( audioSinkBuffer ), GST_BUFFER_SIZE( audioSinkBuffer ) );
}

void GStreamerWrapper::audioEosCallback()
{

}

void GStreamerWrapper::setVideoCompleteCallback( const std::function<void(GStreamerWrapper* video)> &func ){
	mVideoCompleteCallback = func;
}



};
