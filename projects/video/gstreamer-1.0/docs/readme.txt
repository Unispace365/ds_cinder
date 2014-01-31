As most or all of you have seen, I've gotten Gstreamer streaming integrated into bigworld and playing on our wall. Yay!
 
Currently, there are no projects in development that require GStreamer in BigWorld, but we expect that to change soon with General Mills, Tata, and maybe Boeing. So until that happens, all of the GStreamer code is living in a separate branch on git. I'd consider all of the GStreamer stuff to be in alpha for now, with some development needed for any production deployment. At some point, the GStreamer branch will be merged into the master branch, and all apps will have the option of using GStreamer playback.
 
Here's all the nitty gritty (in case anyone needs to know while I'm out this month, or me, after I've forgotten everything after vacation):
 
Streaming
- Ability to accept incoming RTP streams in H.264. (Desktop streams have been sent from the command line currently)
- Sync between multiple machines is very good, as frames display as fast as they come in over UDP
- Need to investigate using a hardware encoder for lower latency / better quality playback
- If deployed as a laptop-streaming or PowerPoint-viewing app, will need to build a streaming app on the sending side (unless using a hardware encoder)
- Can stream files (as opposed to sharing a desktop) with good results
- Currently, can only display one sprite per stream. If we need multiple sprites, we'd need to work on a way to share a texture on the RenderEngine
 
Video / Audio File Playback
- Can playback many formats (mov, avi, mp4, h.264, animation, webm, ogg, etc.)
- Good quality playback (generally superior to the CUDA player, especially in regards to black levels)
- Alpha channels are supported (if the codec supports it), but more development is needed on the World / Render to support it
- More limited number of simultaneous videos than CUDA, since playback is all CPU based. Number of videos at a time is heavily dependent on CPU, video resolution, and codec.
- Some development is needed to differentiate unsynchronized playback from synchronized playback and streaming.
- Audio playback without video is supported (mp3, aif, wav, etc)
 
Synchronized Playback
- Supportable, but more development is needed to support this:
                - Assigning master / slaves
                - Distribute playback time to all instances
                - Keep track of ports used, as GStreamer uses its own net connections to sync tracks and ports can only be used once per instance
                - Special support for Video Sprite Group-like capabilities
- Currently, the best way to sync videos in GStreamer is to simply stream the source.
 
Hardware Accelerated Playback
- There are some options out there for GStreamer and hardware acceleration (va-api, directshow is a maybe), but it's unclear if they support Windows or our usage
- Support may be added in the future, potentially allowing more simultaneous video playback.
 
Removing NVidia GPU requirement
- It's possible, but like all things on this list, needs some more development time to be production-ready
- AMD / ATI cards can run a Render Engine if the CUDA context is not initialized and no CUDA video sprites are created
- There are some graphical issues on AMD:
                - Images seem to load in 'bursts', instead of immediately like they're supposed to
                - Some fonts have graphic issues with some letters (the texture looks corrupt, or incorrect)
                - Some small images don't display properly
- In short, if we run into a situation where we absolutely need to support AMD or Intel, we can if we have a week or so to work out the above issues.
 
GStreamer pipelines
- GStreamer is pipeline-based, meaning when you play a video or start a stream, a series of plugins are linked together to play the video back.
- There's a ton of plugins available that do everything from loading files to doing video effects
- This means that in the future, it should be easy to do cool stuff like add video effects (like old-timey aged film), flip video (in case it needs to be projected on glass or something), adjust color, contrast, gamma, saturation, etc, add subtitles, audio filters, and a bunch of other things
 
Compiling
- download and install Gstreamer 1.2.0: http://gstreamer.freedesktop.org/data/pkg/windows/
                - gstreamer-1.0-x86-1.2.0.msi
                - gstreamer-1.0-devel-x86-1.2.0.msi
- Install them with complete packages in the default location.
- Currently, dll's aren't checked into git, so you'll need to copy them yourself. When we go into production, we may want to check in the needed dll's
                - Copy  the contents of "C:\gstreamer\1.0\x86\bin" to "RenderEngine/bin". There will be conflicts. Overwrite the existing versions with the GStreamer versions.
                - Copy the contents of "C:\gstreamer\1.0\x86\lib\gstreamer-1.0" to "RenderEngine/bin/plugins". The plugins are what drive all the different formats and streaming and such.
- Property sheets are setup to link to the default location (c:/gstreamer/1.0/x86/), which should be made into an environment variable at some point
- We'll only be supporting GStreamer 1.0+ in BigWorld, as pre-1.0 GStreamer is no longer being maintained or supported. We should be able to easily keep up with newer GStreamer releases, as they tend to keep the API's the same or similar over time.
- GStreamer video sprite is a separate video sprite from the CUDA one. Since each video system has it's advantages (CUDA can play more simultaneous syncronized videos and GStreamer can stream and play more formats), it makes sense to support both for the time being.
- The API is currently not finished, and there is a lot of stuff missing (audio targetting, volume control, looping, and I'm sure some other stuff). As we build out support for different clients, we'll add to this.
 
Installing and Running
- When deploying an app, there's two paths to go down for the RenderEngine:
                - Using GStreamer: you'll need to include all the dll's listed in the Compliling section, including the plugins. This is about 150MB of dll's.
                - App doesn't use GStreamer (but is compiled against it): you only need to include the dll's in the bin/ folder (not the plugins). Compressed, this is about 30mb of extra space.
- No environment variables need to be set and GStreamer doesn't need to be installed on deployed instances. This will allow us to deploy multiple GStreamer versions if we have to, or allow multiple apps to live on the same machine without GStreamer conflicts (such as Directv, which still uses GStreamer 0.10)
- The RenderEngine looks for the "plugins" folder in the "bin" folder the first time a GStreamer video sprite is loaded.