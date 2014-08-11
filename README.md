Welcome to the new BigWorld framework implemented in Cinder.
------------------------------------------------------------

**Couple of brief notes for getting started:**

 -  You'll need **four** environment variables set:
   - `CINDER` should point to the current cinder dist (cinder_0.8.4_vc2010
   as of this writing)
   - `DS_PLATFORM` should point to this repository.
   - `QUICKTIME` should point to quicktime dist (QuickTimeSDK-7.3 or
   whatever version is needed)
   - `FREETYPE` should point to the included lib in the lib/ folder. For now you'll still need to set this env var, but that should be worked out and it should be fully integrated without the variable.

 -  You should currently start by copying the example/starter as the basis for your new app.  If you do this, everything will be setup correctly and ready to go.  You can stop reading right here! **(if you DON'T do this step. and copy the starter project, then you will need to manually set up the new solution yourself.)**

Property sheets
---------------

We are using Visual Studio property sheets to manage basic settings like libraries and include paths. The property sheets are located in `ds_cinder/vc10/`, `PlatformSetup.props` (release config) and `PlatformSetup_d.props` (debug config).  What this means is that in the future, all include paths, libraries, and library include paths should be added to these property sheets, so that apps will inherit any changes.  All of these values are specified in the property sheets Common Properties->User Macros page.

 1. Open the Property Manager (View -> Property Manager).
 2. For each project, in the Debug config add `PlatformSetup_d.props` and in release add `PlatformSetup.props` (in both cases, right  3. click the config and select Add Existing Property Sheet...).
 4. In C/C++->General set Additional Include Directories to `$(platform_include_d)"` (in debug) or `"$(platform_include)` (in release)
 5. In Linker->General set Additional Library Directories to `$(platform_lib_include_d);%(AdditionalLibraryDirectories)` (in debug) or `$(platform_lib_include);%(AdditionalLibraryDirectories)` (in release)
 6. In Linker->Input set Additional Dependencies to `$(platform_lib_d);%(AdditionalDependencies)` (in debug) or `$(platform_lib);%(AdditionalDependencies)` (in release)
 7. In Linker->Input set Ignore Specific to `$(platform_lib_ignore_d)` (in debug) or `$(platform_lib_ignore)` (in release)

DLLs
----

We are currently making use of ZeroMQ, which requires a DLL to be located in the same folder as the executable.  To automatically copy over the correct files, add a build step.

 1. Open the properties on your project.
 2. Go to Configuration Properties->Build Events->Post Build Event
 3. Set Command line to: `copy /Y "$(DS_PLATFORM)\lib\zmq\lib\*" "$(OutDir)"`
 4. Set Description to something like Copying ZMQ

For using markdown
----
Add AWESOMIUM to the dscinder defines and it will compile the awesomium stuff in. Then copy the files in the "for_markdown" folder to there correct locations.

To compile with GSTREAMER
-------------------------

Install Gstreamer 1.2.3 runtime and development files available from the gstreamer site. Use the x86, not x64 versions.


Troubleshooting installation
--------------------------------

 - if you get errors for `xaudio.h`: install latest DirectX sdk
 - If you get errors for `Gstreamer` and you DO know that you have installed it, make sure you installed on root level of drive `c:\` (make a symbolic link to wherever else you might have installed it incorrectly)
 - If you get errors of missing `Boost stdint` headers: make sure your cinder distribution does include Boost!
