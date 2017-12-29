-----------------------------
DS Cinder Version 104.0.0
-----------------------------

- Supports Cinder 0.9.0
- Reworked rendering of all sprites for new OpenGL stuff
- Switch text system to Pango. No more MultilineText, it's all just Text now
- Lots of incremental changes and fixes
- Logs go to the documents directory now by default
- New common key commands: m to show/hide mouse and a few others
- Linux support
- 64bit support
- Poco update to 1.7.8
- MuPDF update to 1.11
- Supports most recent GStreamer (1.12.4 as of this writing)
- More GStreamer control: run arbitrary pipelines, get GstElements, support multiple audio outputs
- New project generator in the utility folder (run with Powershell)

-----------------------------
DS Cinder Version 103.0.0
-----------------------------

- Add CEF for web display (Chromium Embedded Framework)
- Add an extended keyboard
- Drawing canvas for using touch input to draw
- Lots of incremental changes and fixes

-----------------------------
DS Cinder Version 102.0.0
-----------------------------

- Parallel image loading
- New Layout system and xml sprite loading (See Dynamic Interfaces.md in docs)
- Expanded XML-importing system to allow events, recursive loading, interactivity, and more
- New scritable animation system for easy cascading animations
- Improvements to synchronized video playback
- Removed many exception throws to minimize random crashes
- 360 degree panoramic video playback
- Ability to specify string-labelled colors
- Latest GStreamer support 1.8.2
- Various fixes and improvements

-----------------------------
DS Cinder Version 101.0.0
-----------------------------
The latest and greatest. Rough changes since the first tagged release (100.0.0):

- GStreamer video performance improvements. Now uses a shader for colorspace conversion when possible.
- Add some helper classes for scrolling lists and scroll areas.
- Fixes for perspective mode (touch picking, sprite drawing, etc)
- PDF framework updated
- Some actual documentation! http://update.downstreamdev.com/ds/doc/ds_cinder/
- Added some helpful examples for testing media (media_tester and simple_video_player)
- Added a utility to generate data model classes from a YML file
- Better handling for web errors and callbacks
- Various fixes and improvements