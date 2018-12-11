# GStreamer

## What is it

https://gstreamer.freedesktop.org/

According to its website: GStreamer is a library for constructing graphs of media-handling components. The applications it supports range from simple Ogg/Vorbis playback, audio/video streaming to complex audio (mixing) and video (non-linear editing) processing.

Applications can take advantage of advances in codec and filter technology transparently. Developers can add new codecs and filters by writing a simple plugin with a clean, generic interface.

Basically, it's good at doing media stuff. It can play back video and audio, as well as handle video streaming, writing output, adding effects, and synchronizing over a network.

## Get it and install it now

You probably installed it already for ds_cinder, but if you haven't, get it:

1. Go to https://gstreamer.freedesktop.org/data/pkg/windows/
2. As of this writing, the 1.12.3 version is what you want. Even number minor versions (the 12 in this case) are the stable releases and the ones to use.
3. Download gstreamer-1.0-devel-x86_64-1.12.3.msi
4. Download gstreamer-1.0-x86_64-1.12.3.msi
5. Note: the version numbers will be different if you've selected something else. You want both the "devel" and regular installers, and the x86_64 versions
6. Install both msi installers with the "complete" option
7. If you have any Visual Studio windows open, close them now so they can get the new environment variables set by the gstreamer installers

## Pipelines and components

GStreamer uses pipelines to do anything. Each pipeline is composed of elements. Each element performs a dedicated task, such as reading a file, parsing it's type, decoding video, adding effects, or outputting audio. GStreamer loads plugins dynamically at runtime to determine its capabilities. These plugins are .dll files on Windows, which are scanned when the app starts up.

For the purpose of this guide, That's about all you need to know for now. If you'd like to know more, there's a bevy of documentation on GStreamer's site: https://gstreamer.freedesktop.org/documentation/application-development/index.html

## Tools / utilities

GStreamer comes with a few pre-compiled binaries that can be super helpful when working with GStreamer. They're generally located in c:\gstreamer\1.0\x86_64\bin

More info on tools: https://gstreamer.freedesktop.org/documentation/tutorials/basic/gstreamer-tools.html

### gst-inspect-1.0

gst-inspect gets information about plugins in your gstreamer installation. In a command window, change directories into the location above, then run:

    gst-inspect-1.0.exe
	
Which prints out all plugins installed with gstreamer; useful for if you want to check if you have a valid install or if you want to see whats available while developing. Some other handy commands:

* `gst-inspect-1.0.exe --types audio`: Shows all audio elements. Replace audio with other terms, such as video, effect, filter
* `gst-inspect-1.0.exe --version`: print version information
* `gst-inspect-1.0.exe [plugin or element name]`: info about a specific plugin. Sample elements: playbin, decodebin, autoaudiosink This is great if you're developing a pipeline and want to see the capabilities or variables of an element.
   

### gst-launch-1.0

gst-launch runs a pipeline, helpful if you're developing a pipeline and you want to check if it works before implementing in your app, or if you want to quickly test a file or streaming device. 

    gst-launch-1.0.exe playbin uri=file:///c:/path/to/video.mp4
	
Playbin is an uber element that will automatically parse the input and create whatever elements it needs to to open, decode, and display the media. You can also use playbin to test streaming pipelines:

    gst-launch-1.0.exe playbin uri=rtsp://192.168.0.136:5045/live.sdp
	
Basic pipeline syntax:

    [element] ! [element] ! [element]

Exclamation marks separate elements. You can also specify properties:

    [element] [property]=[property_value]
	
Test pipeline:

    gst-launch-1.0.exe videotestsrc ! videoconvert ! autovideosink
	
videotestsrc creates some fake video frames. videoconvert converts types of video. autovideosink displays video, automatically detecting the best kind of display. Using the auto element types is great for cross-platform compatibility.


### gst-device-monitor-1.0

If you're using a webcam, this will print out info about any connected webcams. This is super helpful for figuring out what video types your webcam uses.

### gst-discoverer-1.0

Use this on a file (using the file:///[path] syntax) to display info about the file, such as codec, framerate, frame size, etc.


## Video playback

### GstVideo

In ds_cinder, the most common use for GStreamer is to play video files located on your harddrive. If you just want raw video playback without a video interface (play/pause button, scrub bar, volume control), use the GstVideo sprite. This is the same as using ds::ui::Video. Create a new video sprite, load the video, hit play, add the sprite to a parent, and you're good to go:

```cpp
	mVideo = new ds::ui::GstVideo(mEngine);
	mVideo->setLooping(true);
	mVideo->loadVideo(mediaPath);
	addChildPtr(mVideo);
```
	
### VideoPlayer and VideoInterface

VideoPlayer is a convenience that sets up a video for you and optionally adds a video interface (play/pause button, scrub bar, volume control). Use VideoPlayer if you are ONLY playing videos and want user control.

```cpp
	mVideoPlayer = new VideoPlayer(mEngine, mEmbedInterface);
	addChildPtr(mVideoPlayer);
	mVideoPlayer->setMedia(absoluteMediaPath);
```

### MediaPlayer

MediaPlayer is an uber player that will automatically determine the media type and load the relevant interface and player for you. For instance, if you want to show a PDF, website or video, you could create a single MediaPlayer and and simply set the resource. MediaPlayer can be resized, and it will automatically fit it's content into it.

### MediaViewer

MediaViewer wraps up MediaPlayer into a Viewer, which is a BasePanel. This lets the user move and size the player dynamically, while also adding an interface. If you don't need to allow the user to dynamically resize the viewer, than it's best to use MediaPlayer.



## Synchronized playback over a network

Setup your app as a server/client and play video and ds_cinder will automatically keep the video playback in sync over the network. Note that you need to have the video in the same location on both machines for this to work. You also need to have a clientserver (and not a plain server) and the video needs to load and play there.

If you'd like the app to only play back videos on some clients, but not all, you can use setPlayableInstances() on GstVideo. Using this may interact poorly with the synchronization system, so use with caution. The primary use case for this is a system that has many clients that are all playing independent large videos, and you don't want to slow playback by loading unneccessary videos.


## Streaming

### StreamPlayer

### Sample pipelines




## Arbitrary pipelines


## Audio routing
