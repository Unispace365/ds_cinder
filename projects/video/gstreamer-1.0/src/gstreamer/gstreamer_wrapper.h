#pragma once

#include <gst/gst.h>
#include <gst/gstbin.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>

#include <mutex>
#include <atomic>
#include <string>
#include <functional>

namespace gstwrapper
{

// Enumeration to describe the current state of the wrapper
enum PlayState
{
	NOT_INITIALIZED,
	OPENED,
	PLAYING,
	PAUSED,
	STOPPED
};
	
enum GstPlayState
{
	STATE_NULL,
	STATE_READY,
	STATE_PAUSED,
	STATE_PLAYING
};

// Enumeration to describe the play direction of the loaded file
enum PlayDirection {
	FORWARD = 1,
	BACKWARD = -1
};

/*
Enumeration to describe how the wrapper should behave once the end of a file has been reached
NO_LOOP --> simply stop
LOOP --> seek back to the start of the file and play again
BIDIRECTIONAL_LOOP --> play the file again from the position where the stream ended and change the play direction
*/
enum LoopMode {
	NO_LOOP,
	LOOP,
	BIDIRECTIONAL_LOOP
};

/*
Enumeration to describe what kind of file has been loaded, which is important to know which buffers (video / audio) contain information or not
VIDEO_AND_AUDIO --> loaded file contains both at least one video and one audio stream
VIDEO --> loaded file contains at least one video stream but no audio streams
AUDIO --> loaded file contains at least one audio stream but no video streams
*/
enum ContentType {
	NONE,
	VIDEO_AND_AUDIO,
	VIDEO,
	AUDIO
};

/*
Enumeration to describe the byte order (big endian = 4321 / little endian = 1234) of either video or audio stream
*/
enum Endianness {
	BIG_ENDIAN = 4321,
	LITTLE_ENDIAN = 1234
};

/*
class GStreamerWrapper

Class that provides the functionality to open any kind of media file (movie and sound files) and the possibility to interact with the file
(play, stop, pause, seek, playing speed / direction, loop modes). Furthermore the user has direct access to both the video and audio buffers
that are respectively stored in an unsigned char pointer. Also, various information regarding the media file can be queried (video size, audio
channels, number of video / audio streams and so on)

This Wrapper is based on the GStreamer library, which means all the decoding and synchronization of the media files is done internally by that library.
*/
class GStreamerWrapper	{
public:
	// Constructor that initializes GStreamer
	GStreamerWrapper();

	// Destructor which closes the file and frees allocated memory for both video and audio buffers as well as various GStreamer references
	virtual ~GStreamerWrapper();


	void debugAppsinkShaderColorspaceOpen();
	/*
	Opens a file according to the string parameter. Sets the wrapper's PlayState to OPENED

	params:
	@strFilename: The filepath to the desired file; Note: It has to be in a URI format, means it always begins with
	"file:/" or "file:///" and only contains absolute paths, relative paths do not work
	example: "file:/C:/MyFolder/myFile.mp4" or "file:///C:/MyFolder/myFile.mp4"

	@generateVideoBuffer: If true, the user can manually retrieve the video buffer via the getVideo() method. If false, no
	video buffer will be generated which means getVideo() will return NULL. Instead, GStreamer will open a new video player window
	by itself and will render the video inside that new window while automatically choosing the appropriate video codec

	@generateAudioBuffer: If true, the user can manually retrieve the audio buffer via the getAudio() method. If false, no
	audio buffer will be generated which means getAudio() will return NULL. Instead, GStreamer will choose an appropriate
	audio codec of the operating system and play the sound synchronized to the video (or just play the sound if there is no video
	data)

	@ transparent: If true, will set the bits per pixel to 32 and generate alpha values, even if the source video doesn't have them (all opaque in that case).
			 
	@ videoWidth: Specify the size of the video. Required before creating a pipeline
	@ videoHeight: Specify the size of the video. Required before creating a pipeline
	*/
	bool					open( std::string strFilename, bool bGenerateVideoBuffer, bool bGenerateAudioBuffer, bool isTransparent, int videoWidth, int videoHeight);

	/*
	Closes the file and frees allocated memory for both video and audio buffers as well as various GStreamer references
	*/
	virtual void			close();

	/*
	Updates the internal GStreamer messages that are passed during the streaming process. This method is also needed to detect
	whether a stream is finished or not or if the stream encountered any errors
	*/
	void					update();

	/*
	Play the previously opened media file. Sets the wrapper's PlayState to PLAYING
	*/
	void					play();

	/*
	Stop the media file. If the file is played again it will start from either the beginning (PlayDirection = FORWARD) or the end
	(PlayDirection = BACKWARD). Sets the wrapper's PlayState to STOPPED
	*/
	void					stop();

	/*
	Pauses the media file. If the file is played again the file resumes from exactly the position where it was paused.
	Sets the wrapper's PlayState to PAUSED
	*/
	void					pause();

	/*
	Sets the current video stream

	params:
	@currentVideoStream: The index of the video stream. If the value is either negative or greater than the number of available video
	streams nothing will happen
	*/
	void					setCurrentVideoStream( int iCurrentVideoStream );

	/*
	Sets the current audio stream. A media file might, for example, contain multiple languages.

	params:
	@currentAudioStream: The index of the audio stream. If the value is either negative or greater than the number of available audio
	streams nothing will happen
	*/
	void					setCurrentAudioStream( int iCurrentAudioStream );

	/*
	Sets the playback speed of the opened media file.

	params:
	@fSpeed: Describes the new playback speed of the media file. 1.0f is the default playback speed. Negative values are converted to 0.0f, which means
	this method does not change the playing direction, only the speed. If the playback direction should be changed, use the changeDirection() method instead
	*/
	void					setSpeed( float fSpeed );

	/*
	Sets the playback direction of the opened media file.
	Note: The functionality of this method depends highly on the file format and encoding. For example, some video formats get stuck while playing them backwards
	(happened with several .mp4 files) while other formats have no problem with it at all (tested some .mov files)

	params:
	@direction: The new playback direction, possible values are FORWARD or BACKWARD
	*/
	void					setDirection( PlayDirection direction );

	/*
	Sets the loop mode of the wrapper, which decides how the wrapper should behave once it has reached the end of a stream.
	See the LoopMode enumeration description to see which value triggers what behavior.
	Note: As mentioned in the description of the changeDirection() method, some video formats have difficulties when being
	played backwards. This only affects the BIDIRECTIONAL_LOOP, the other two work fine for all formats

	params:
	@loopMode: The desired loop mode, possible values are NO_LOOP, LOOP and BIDIRECTIONAL_LOOP
	*/
	void					setLoopMode( LoopMode loopMode );

	/*
	Seeks the media file to the desired frame

	params:
	@iTargetFrameNumber: The frame number of the media file where the wrapper should seek to. Negative values as well as values
	that are greater than the number of frames are being ignored and nothing happens
	*/
	void					setFramePosition( gint64 iTargetFrameNumber );

	/*
	Seeks the media file to the position provided by the milliseconds parameter

	params:
	@dTargetTimeInMs: The desired position in milliseconds where the wrapper should seek to. Negative values as well as values
	that are greater than the media duration in milliseconds are being ignored and nothing happens
	*/
	void					setTimePositionInMs( double dTargetTimeInMs );

	/*
	Seeks the media file to the position provided by the nanoseconds parameter

	params:
	@dTargetTimeInNs: The desired position in nanoseconds where the wrapper should seek to. Negative values as well as values
	that are greater than the media duration in nanoseconds are being ignored and nothing happens
	*/
	void					setTimePositionInNs( gint64 iTargetTimeInNs );

	/*
	Seeks the media file to the position provided by a percentage statement between 0 and 100 percent.
	0 percent means the beginning of the file, 50 percent the middle and 100 percent the end of the file.

	params:
	@fPos: The percentage value between 0.0f (= 0 percent) and 1.0f (= 100 percent). Values that are negative or greater than 1.0f
	are being clamped to 0.0f and 1.0f respectively
	*/
	void					setPosition(double fPos);


	/*
	Returns true if the loaded media file contains at least one video stream, false otherwise
	*/
	bool					hasVideo();

	/*
	Returns true if the loaded media file contains at least one audio stream, false otherwise
	*/
	bool					hasAudio();

	/*
	Returns the URI of the currently opened media. If no file has been opened yet an empty string is returned
	*/
	std::string				getFileName();

	/*
	Returns an unsigned char pointer containing the pixel data for the currently decoded frame.
	Returns NULL if there is either no video stream in the media file, no media file has been opened or something
	went wrong while streaming
	*/
	unsigned char*			getVideo();

	/*
	Returns the index of the current video stream
	*/
	int						getCurrentVideoStream();

	/*
	Returns the number of available video streams
	*/
	int						getNumberOfVideoStreams();

	/*
	Returns the index of the current audio stream
	*/
	int						getCurrentAudioStream();

	/*
	Returns the number of available audio streams
	*/
	int						getNumberOfAudioStreams();

	/*
	Returns the video width
	*/
	unsigned int			getWidth();

	/*
	Returns the video height
	*/
	unsigned int			getHeight();

	/*
	Returns the Frames Per Second of the current video
	*/
	float					getFps();

	/*
	Returns if the buffer you get with getVideo holds really a new image of the video, use this to increase performance in your applications, so you don't unnecessary copy mem to textures
	*/
	bool					isNewVideoFrame();


	/*
	Returns the current playback speed value
	*/
	float					getSpeed();

	/*
	Returns the current percentaged position of the stream which is a value between 0.0f (= 0 percent) and 1.0f (= 100 percent)
	*/
	double					getPosition() const;

	/*
	Returns the current frame number of the stream
	*/
	gint64					getCurrentFrameNumber();

	/*
	Returns the number of frames of the stream
	*/
	gint64					getNumberOfFrames();

	/*
	Returns the current position of the stream in milliseconds
	*/
	double					getCurrentTimeInMs();

	/*
	Returns the duration of the stream in milliseconds
	*/
	double					getDurationInMs() const;

	/*
	Returns the current position of the stream in nanoseconds
	*/
	gint64					getCurrentTimeInNs() const;

	/*
	Returns the duration of the stream in nanoseconds
	*/
	gint64					getDurationInNs();

	/*
	Returns the current PlayState
	*/
	PlayState				getState() const;

	/*
	Returns the current playback direction
	*/
	PlayDirection			getDirection();

	/*
	Returns the current LoopMode
	*/
	LoopMode				getLoopMode();

	/*
	Returns type of content of the loaded media file
	*/
	ContentType				getContentType();

	/*
	Sets the Pipeline volume

	params:
	@fVolume: The new volume value which will be immediately applied to the Pipeline. Any value between 0.0f and 1.0f are possible.
	Negative values will be clamped to 0.0f and values greater than 1.0f to 1.0f.
	*/
	void					setVolume( float fVolume );

	/*
	Returns an unsigned char pointer containing a buffer to the currently decoded audio data
	Returns NULL if there is either no audio stream in the media file, no file has been loaded or something went wrong
	while streaming
	*/
	unsigned char*			getAudio();

	/*
	Returns true if the audio stream is signed, false otherwise
	*/
	bool					getIsAudioSigned();

	/*
	Returns the number of available audio channels
	*/
	int						getNumOfAudioChannels();

	/*
	Returns the audio sample rate
	*/
	int						getAudioSampleRate();

	/*
	Returns the audio buffer size
	*/
	int						getAudioBufferSize();

	/*
	Returns the audio buffer size without the audio width and audio channels
	*/
	int						getAudioDecodeBufferSize();

	/*
	Returns the audio width (8, 16, 24 or 32)
	*/
	int						getAudioWidth();

	/*
	Returns the current volume value
	*/
	float					getCurrentVolume();

	/*
	Returns the Endianness of the audio stream
	*/
	Endianness				getAudioEndianness();


	/*
	Lamda is called when GStreamer gets an EOS message (not called when looping)
	*/
	void					setVideoCompleteCallback(const std::function<void(GStreamerWrapper* video)> &func);

	/*
	Set the pipeline to play as soon as the video is loaded.
	*/
	void					setStartPlaying(const bool startPlaying){ m_StartPlaying = startPlaying; }

	/*
	Stop the pipeline at the end of the current loop (if looping) or on End of Stream.
	This is a single shot, so as soon as the video get's stopped, it can be looped again.
	*/
	void					stopOnLoopComplete(){ m_StopOnLoopComplete = true; };


	//Custom pipeline function call
	virtual void			setCustomFunction(){};

	void					enableCustomPipeline(bool enable) { m_CustomPipeline = enable; }
	
	/*
	Here the GStreamer messages are read and processed. Needed for error checking while streaming and
	detecting if a stream has reached the end
	*/
	virtual void			handleGStMessage();

	/*
	Seeks to the desired position in the media file using nanoseconds

	params:
	@iTargetTimeInNs: Time position in nanoseconds
	*/
	bool					seekFrame(gint64 iTargetTimeInNs);
	/*
	Retrieves all needed media information such as duration, video size and frame rate
	*/
	void					retrieveVideoInfo();

	/** Spite out a ton of messages when running gstreamer pipelines. */
	void					setVerboseLogging(const bool verboseOn);


	bool					getSharedDrawable();
	int						getSharedTextureId();

	void					setSharedParams(const bool drawable, const int textureId);

private:
	/*
	Helper method in order to apply either changes to the playback speed or direction in GStreamer
	*/
	bool					changeSpeedAndDirection( float fSpeed, PlayDirection direction );

	/*
	GStreamer callback method that is called when the Pipeline is set to a paused state. Through the appsink
	the buffer can be gathered. Furthermore, the unsigned char array for the pixel data is allocated here if
	it hasn't been allocated yet

	params:
	@appsink: The video appsink --> needed to get the video buffer while the stream is in a paused state

	@listener: Reference to an instance of an class, needed for additional calls ("this" in this case)
	*/
	static GstFlowReturn	onNewPrerollFromVideoSource( GstAppSink* appsink, void* listener );

	/*
	GStreamer callback method that is called when the Pipeline is set to a paused state. Through the appsink
	the buffer can be gathered. Furthermore, the unsigned char array for the audio data is allocated here if
	it hasn't been allocated yet

	params:
	@appsink: The audio appsink --> needed to get the audio buffer while the stream is in a paused state

	@listener: Reference to an instance of an class, needed for additional calls ("this" in this case)
	*/
	static GstFlowReturn	onNewPrerollFromAudioSource( GstAppSink* appsink, void* listener );

	/*
	GStreamer callback method that is called whenever there is a new video frame decoded. Through the appsink
	the buffer, which contains the pixel data, can be gathered

	params:
	@appsink: The video sink --> needed to get the video buffer

	@listener: Reference to an instance of an class, needed for additional calls ("this" in this case)
	*/
	static GstFlowReturn	onNewBufferFromVideoSource( GstAppSink* appsink, void* listener );

	/*
	GStreamer callback method that is called whenever there is a new chunk of audio data decoded. Through the appsink
	the buffer, which contains the audio data, can be gathered

	params:
	@appsink: The audio sink --> needed to get the audio buffer

	@listener: Reference to an instance of an class, needed for additional calls ("this" in this case)
	*/
	static GstFlowReturn	onNewBufferFromAudioSource( GstAppSink* appsink, void* listener );


	/*
	Non-static method that is called inside "onNewPrerollFromVideoSource()" in order to handle
	member variables that are non-static. Here the unsigned char array with the pixel data is actually filled
	and also allocated if is NULL

	params:
	@videoSinkBuffer: The buffer that was gathered from the video sink
	*/
	void					newVideoSinkPrerollCallback( GstSample* videoSinkBuffer );

	/*
	Non-static method that is called inside "onNewBufferFromVideoSource()" in order to handle
	member variables that are non-static. Here the unsigned char array with the pixel data is actually filled

	params:
	@videoSinkBuffer: The buffer that was gathered from the video sink
	*/
	void					newVideoSinkBufferCallback( GstSample* videoSinkBuffer );

	/*
	Non-static method that is called inside "onNewPrerollFromAudioSource()" in order to handle
	member variables that are non-static. Here the unsigned char array with the audio data is actually filled
	and also allocated if it is NULL

	params:
	@audioSinkBuffer: The buffer that was gathered from the video sink
	*/
	virtual void			newAudioSinkPrerollCallback( GstSample* audioSinkBuffer );

	/*
	Non-static method that is called inside "onNewBufferFromAudioSource()" in order to handle
	member variables that are non-static. Here the unsigned char array with the audio data is actually filled

	params:
	@audioSinkBuffer: The buffer that was gathered from the audio sink
	*/
	virtual void			newAudioSinkBufferCallback( GstSample* audioSinkBuffer );

	// Getting app callbacks for new sinks requires registereing EOS callbacks.
	// However, we handle EoS events from the message bus, so these don't do anything, but don't delete them.
	static void				onEosFromVideoSource(GstAppSink* appsink, void* listener);
	static void				onEosFromAudioSource(GstAppSink* appsink, void* listener);


protected:

	int						m_iAudioBufferSize; /* Size of the audio buffer */
	unsigned char*			m_cAudioBuffer; /* Stores the audio data */
	int						m_iAudioWidth; /* Width of the audio data (8, 16, 24 or 32) */
	bool					m_bIsAudioSigned; /* Flag that tracks if the audio buffer is signed or not */
	Endianness				m_AudioEndianness; /* Audio endianness, either big or small endian */
	int						m_iAudioDecodeBufferSize; /* Size of the audio buffer without the channels and audio width */
	int						m_iNumAudioChannels; /* Number of audio channels */
	int						m_iAudioSampleRate; /* Audio sample rate */
	GstBus*					m_GstBus; /* The pipeline's bus, needed to track messages that are passed while streaming */
	GstMessage*				m_GstMessage; /* Message gathered from the bus */
	gint64					m_PendingSeekTime;
	GstPlayState			m_CurrentGstState; /* the current state of Gstreamer */
	bool					m_StopOnLoopComplete; /* Set the pipeline to NULL_STATE (Stopped) on the end of the current loop or on EOS */
	float					m_fSpeed; /* The current playback speed */
	gint64					m_iDurationInNs; /* Duration of media file in nanoseconds */
	LoopMode				m_LoopMode; /* The current loop mode */
	std::function < void(GStreamerWrapper*) >
							mVideoCompleteCallback;
	GstElement*				m_GstPipeline; /* The main GStreamer pipeline */
	bool					m_PendingSeek;
	GstElement*				m_GstAudioSink; /* Audio sink that contains the raw audio buffer. Gathered from the pipeline */

private:

	std::mutex				m_VideoMutex;
	std::unique_lock < std::mutex >
							m_VideoLock;
	bool					m_bFileIsOpen; /* Flag that tracks if a file has been opened or not */
	std::atomic<bool>		m_bIsNewVideoFrame; /* Flag that tracks if there is actually a new frame or not */
	std::string				m_strFilename; /* Stores filepath of the opened media file */
	std::string				m_strCodecName;
	int						m_iCurrentVideoStream; /* Index of the current video stream */
	int						m_iNumVideoStreams; /* Number of available video streams */
	int						m_iCurrentAudioStream; /* Index of the current audio stream */
	int						m_iNumAudioStreams; /* Number of available audio streams */
	int						m_iWidth; /* Video width */
	int						m_iHeight; /* Video height */
	int						m_iBitrate; /* Video bitrate */
	float					m_fVolume; /* Volume of the pipeline */
	float					m_fFps; /* Frames per second of the video */
	double					m_dCurrentTimeInMs; /* Current time position in milliseconds */
	double					m_dDurationInMs; /* Media duration in milliseconds */
	gint64					m_iCurrentFrameNumber; /* Current frame number */
	gint64					m_iNumberOfFrames; /* Total number of frames in media file */
	mutable gint64			m_iCurrentTimeInNs; /* Current time position in nanoseconds */
	PlayState				m_CurrentPlayState; /* The current state of the wrapper */
	PlayDirection			m_PlayDirection; /* The current playback direction */
	ContentType				m_ContentType; /* Describes whether the currently loaded media file contains only video / audio streams or both */
	unsigned char*			m_cVideoBuffer; /* Stores the video pixels */
	GstElement*				m_GstVideoSink; /* Video sink that contains the raw video buffer. Gathered from the pipeline */
	GstAppSinkCallbacks		m_GstVideoSinkCallbacks; /* Stores references to the callback methods for video preroll, new video buffer and video eos */
	GstAppSinkCallbacks		m_GstAudioSinkCallbacks; /* Stores references to the callback methods for audio preroll, new audio buffer and audio eos */
	bool					m_StartPlaying;/* Play the video as soon as it's loaded */
	bool					m_CustomPipeline;

	bool					m_VerboseLogging;

	int						m_SharedTextureId;
	bool					m_SharedDrawable;


}; //!class GStreamerWrapper
}; //!namespace gstwrapper
