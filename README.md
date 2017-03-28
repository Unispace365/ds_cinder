DS Cinder
=========
DS Cinder is a framework for interactive applications built on top of the fantastic Cinder framework. DS Cinder provides conveniences for getting graphics onscreen; loading data from a sqlite database; displaying PDFs, videos and web pages; providing touch interaction; and loading settings. The DS version also provides the ability to syncronize multiple clients to display the same graphics, as well as conveniences for touch handling.


-----------------------------
DS Cinder - Transition to Cinder 0.9
-----------------------------
Lots of things have changed in both Cinder & openGL since Cinder .84. The documentation has some useful pages for explaining what has changed which can be found ([Here](https://libcinder.org/docs/guides/transition_0_9/index.html)) and ([Here](https://libcinder.org/docs/guides/opengl/index.html)).
- Major changes
    - Cinder moved to GLM for Vectors and Matricies (`ci::Vec3f => ci::vec3`). That also means that instead of writing `MyVec.normalize()` it would be `MyVec = normalize(MyVec)`, which matches the glsl conventions.
    - Instead of using ci::gl::Texture et. al. the new convention is to use ci::gl::TextureRef, which is a shared pointer to the texture.
    - Many openGL functions have been depricated or removed with the removal of "Immediate Mode" rendering. Cinder provides some replication of the old functionality,but things like ALPHA_TEST, will need to be replaced with equivalent shader(s).
    - Things like `ds::SaveCamera` & `ds::SaveViewport` can be replaced with Cinders new `ci::Scoped*` group of classes, which follow the same RAII structre.
    - Cinder now has a set of default shaders that can replace the old default "Immediate Mode", as well as ones that can do simple 3d lighting.
- Current Progress:
    - Created 090_develop branch
    - Converted most of the old style vector and matrix notations

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

----------

Installation
------------
!! For Cinder 0.9, add the environment variable CINDER_090 and point it to the unziped 0.90 cinder distribution folder !!
This version of DS Cinder requires **Visual Studio 2013** installed.

-  You'll need some environment variables set:
   - CINDER_090 points to the unzipped 0.8.6 cinder distribution folder ([Version 0.9.0 for VC 2013](https://libcinder.org/static/releases/cinder_0.9.0_vc2013.zip)).

   - DS_PLATFORM_090 points to the root of this repository.


Use the project generator to create a new project. This will make a copy of the "full_starter" example project, which contains all the projects you'll need to get started (PDF, Video, etc) and a bare-bones app structure.

----------

Troubleshooting installation
--------------------------------

 - If you get errors for `xaudio.h`: install [latest DirectX SDK][2]
 - If you get errors of missing `Boost cstdint` headers: make sure your cinder distribution does include Boost! Be sure to use the download on the cinder home page, and not a release from the cinder github.
 - If you get `LNK1123: failure during conversion to COFF: file invalid or corrupt'`: Install latest update for your Visual Studio!
 - `SerialRunnable`: You may need to pass an alloc function when initializing a SerialRunnbale.
 - `boost::mutex` to `std::mutex`. In most cases for threading, the `boost` versions are supplanted with the `STL` version. Check stack overflow / google, there's plenty of upgrade examples
 - `Not defined`s: Many `STL` elements now need to have `include`s, most commonly `<memory>`, `<cctype>` and `<sstring>`.
 - `KeyEvent Not Defined`: Since the removal of using namespace `ci::*` from `ds_cinder` files, you'll need to make sure everything is namespaced properly.
 - Be sure you have the correct environment variables. Clean the solution. Restart visual studio and/or your machine

----------


Property sheets
---------------

We are using **Visual Studio property sheets** to manage basic settings like libraries and include paths. The property sheets are located in `ds_cinder/vs2013/PropertySheets`, `PlatformSetup.props` (release config) and `PlatformSetup_d.props` (debug config).  What this means is that in the future, all include paths, libraries, and library include paths should be added to these property sheets, so that apps will inherit any changes.  All of these values are specified in the property sheets Common Properties->User Macros page.

 1. Open the Property Manager (`View -> Property Manager`).
 2. For each project, in the Debug config add `PlatformSetup_d.props` and in release add `PlatformSetup.props` (in both cases, right  3. click the config and select Add Existing Property Sheet...).
 4. In `C/C++ -> General` set `Additional Include Directories` to `$(platform_include_d)"` (in debug) or `"$(platform_include)` (in release)
 5. In `Linker -> General` set `Additional Library Directories` to `$(platform_lib_include_d);%(AdditionalLibraryDirectories)` (in debug) or `$(platform_lib_include);%(AdditionalLibraryDirectories)` (in release)
 6. In `Linker -> Input` set Additional Dependencies to `$(platform_lib_d);%(AdditionalDependencies)` (in debug) or `$(platform_lib);%(AdditionalDependencies)` (in release)
 7. In `Linker -> Input` set `Ignore Specific` to `$(platform_lib_ignore_d)` (in debug) or `$(platform_lib_ignore)` (in release)



To compile with GSTREAMER
-------------------------

Install **Gstreamer 1.8.2** runtime and development files. Use the x86, not x64 versions. Select "complete" when installing. These installers should create an environment variable for GSTREAMER_1_0_ROOT_X86 which points to the root gstreamer directory, typically at c:/gstreamer/1.0/x86/
  - [Runtime installer](http://gstreamer.freedesktop.org/data/pkg/windows/1.8.2/gstreamer-1.0-x86-1.8.2.msi)
  - [Development files installer](http://gstreamer.freedesktop.org/data/pkg/windows/1.8.2/gstreamer-1.0-devel-x86-1.8.2.msi)


