DS Cinder
=========
BigWorld framework implemented in Cinder.

----------

Installation
------------
This version of DS Cinder requires **Visual Studio 2013** installed.

 -  You'll need some environment variables set:
   - `CINDER_086` should point to the 0.8.6 cinder dist ([Version 0.8.6 for VC 2013](http://libcinder.org/releases/cinder_0.8.6_vc2013.zip)). e.g:

     ```Batchfile
     setx CINDER %USERPROFILE%\code\cinder_0.8.6_vc2013
     ```

   - `DS_PLATFORM_086` should point to this repository. e.g:

     ```Batchfile
     setx DS_PLATFORM %USERPROFILE%\code\ds_cinder
     ```

   - ***[optional]*** `DS_CINDER_GSTREAMER_1-0` should point to Gstreamer (if your project uses Gstreamer). e.g:

     ```Batchfile
     setx DS_PLATFORM c:/gstreamer/1.0/x86/
     ```

 -  You should start by copying one of the example projects located in the `examples` folder as the basis for your new app.  If you do this, everything will be setup correctly and ready to go.  You can stop reading right here! **(if you DON'T do this step and copy the starter project, then you will need to manually set up the new solution yourself.)**

----------

Troubleshooting installation
--------------------------------

 - If you get errors for `xaudio.h`: install [latest DirectX SDK][2]
 - If you get errors of missing `Boost cstdint` headers: make sure your cinder distribution does include Boost!
 - If you get `LNK1123: failure during conversion to COFF: file invalid or corrupt'`: Install latest update for your Visual Studio!
 - `SerialRunnable`: You may need to pass an alloc function when initializing
 - `boost::mutex` to `std::mutex`. In most cases for threading, the `boost` versions are supplanted with the `STL` version. Check stack overflow / google, there's plenty of upgrade examples
 - `Not defined`s: Many `STL` elements now need to have `#include`s, most commonly `<memory>`, `<cctype>` and `<sstring>`.
 - `KeyEvent Not Defined`: Since the removal of using namespace `ci::*` from `ds_cinder` files, you'll need to make sure everything is namespaced properly.

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


----------

DLLs
----

We are currently making use of ZeroMQ, which requires a DLL to be located in the same folder as the executable.  To automatically copy over the correct files, add a build step.

 1. Open the properties on your project.
 2. Go to Configuration `Properties -> Build Events -> Post Build Event`
 3. Set Command line to: `copy /Y "$(DS_PLATFORM)\lib\zmq\lib\*" "$(OutDir)"`
 4. Set Description to something like `Copying ZMQ`


----------


To compile with GSTREAMER
-------------------------

Install **Gstreamer 1.2.3** runtime and development files available [from the gstreamer site][1] and make sure you have Gstreamer's environment variable defined (*refer to installation section of this document*). Use the x86, not x64 versions.
  - [Runtime installer](http://gstreamer.freedesktop.org/data/pkg/windows/1.2.3/gstreamer-1.0-x86-1.2.3.msi)
  - [Development files installer](http://gstreamer.freedesktop.org/data/pkg/windows/1.2.3/gstreamer-1.0-devel-x86-1.2.3.msi)

----------


  [1]: http://gstreamer.freedesktop.org/data/pkg/windows/
  [2]: http://lmgtfy.com/?q=directx%20sdk%20download
  
