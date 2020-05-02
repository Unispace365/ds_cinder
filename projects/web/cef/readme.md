# CEF Web Sprites

Chromium Embedded Framework is a wrapper for Chromium, https://bitbucket.org/chromiumembedded/cef , which is the core of Chrome, a little-known web browser put out by a small California startup called "Google." CEF allows for much more control over the underlying browser and since the entire stack is open-source, the ability to fix any issues that come up. It's also up-to-date, meaning some small time websites, like google.com, actually function like they're supposed to and don't show security warnings. Additionally, more advanced functionality like that of Google Drive work properly.

## Features

* Open source for maintainability
* Recent version of Chromium core, and ability to stay up-to-date in the future
* Functional HTML5 support
* More callbacks from the browser: fullscreen toggle, loading, title changes, errors, etc.
* Better performance with multi-process setup
* Security
* PDF viewing
* Customizeable error page
* Mouse / Touch / Keyboard input


### Still to-be-implemented in ds_cinder

* Find handling
* Drag-n-drop media (intra-browser dragging works)
* Geolocation should be working, but appears to be blocked somewhere
* Multi-computer synchronization
* Javascript input/output
* Javascript pop-up alert dialogs
* Favicons
* Downloads
* Context menus
* File dialogs
* Tool tips
* Status messages
* Console messages
* Field focus changes (for advanced controls of forms)
* Cursor changes
* Scroll offset changes


## CEF Process and threading model

Be sure to check all the docs and comments thoroughly before making any changes.

### Lifecycle and threading

* On startup, WebService initializes CEF. This creates a WebApp (manages the context) and WebHandler (deals with browser-to-sprite communication and requests)
* The ds_cinder main app thread creates a Web sprite.
* The WebService requests WebApp to create a new CefBrowser on the main thread. A browser is a single unit of web stuff, like a tab in Chrome.
* When the browser has been created asynchronously, it calls back to the main thread's Web sprite to notify it of the browser's identifier. 
* From there, the Web sprite registers callbacks for updated buffers, load state changes, title changes, etc. 
* Whenever the Web Sprite gets a callback or deals with any data shared with the browser, it must lock the main thread, as all callbacks from CEF come from CEF's UI thread. (CEF uses many threads, and the UI thread is the primary one)
* Whenever the Web Sprite makes a request of the browser (forwards, back, close, load, etc), the WebHandler must lock CEF, so updates to the same map/vector don't happen at the same time. 

### Processes

CEF requires multiple processes for efficiently and securely loading web sites. Since the main ds_cinder process / executable handles making windows and consoles before any client code, we use a separate executable for the additional CEF processes. This executable is an extremely bare-bones CEF implementation based on SimpleApp, which is shipped with CEF binary distributions. cefsimple.exe is required to be in the working directory for this to function. Luckily, that exe is copied automatically by the cef_web build process. 

### Resources

CEF requires a number of resources to run. These are pak, bin, dll, dat and locale files. They're all included in ds_cinder/projects/web/cef/lin/runtime. Any updates to CEF should be sure to make sure that there are the proper resources in this folder. Everything in the runtime folder gets copied to the $Configuration folder at compile time.

## Updating or recompiling

* Background: I recommend reading through these guides before embarking. We'll use a modified version of these below
    * https://bitbucket.org/chromiumembedded/cef/wiki/MasterBuildQuickStart
    * https://bitbucket.org/chromiumembedded/cef/wiki/BranchesAndBuilding
    * https://bitbucket.org/chromiumembedded/cef/wiki/AutomatedBuildSetup
    * Compiling CEF also compiles Chromium and all of it's third party libraries. This is mostly and automated process but be prepared to wait a while for the full compilation
    * There are pre-existing builds of CEF available, however they don't include proprietary codec support for h.264 and mp3 playback, so we have to compile the whole thing
* Pre-requisites
    * Visual Studio 2017 Community 15.7.1+ installed in the default location
    * Windows 10.0.18362 SDK installed in the default location. https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk You must install this exact SDK version to avoid build issues.
    * At least 8GB of RAM and 40GB of free disk space.
    * Approximately 2 hours with a fast internet connection (25Mbps) and fast build machine (2.6Ghz+, 4+ logical cores).
    * Python 2.7 (I used 2.7.17) 64bit installed in the default location and included in PATH
    * CMake
* Compiling (use this guide instead of MasterBuildQuickStart or BranchesAndBuilding above)
    * Create the directory c:/code (the directory needs to be at the root level to avoid issues with path length down the road)
    * Copy the python script from here: https://bitbucket.org/chromiumembedded/cef/raw/master/tools/automate/automate-git.py and create automate-git.py in c:/code
    * Get the bat file to start compilation
        * Copy build_cef.bat from ds_cinder/projects/web/cef/build/ to c:/code
        * Change --branch=3945 to the current supported release branch from BranchesAndBuilding
    * Run build_cef.bat from c:/code
    * NOTE: You might have to add c:/code/dt/ to your PATH
    * NOTE: I had to manually install pywin32 using this command: "python -m pip install pywin32" See https://magpcss.org/ceforum/viewtopic.php?f=6&t=17258
* Wait
    * More waiting
    * Deal with any compile issues
    * Restart the compilation
    * Waiting
* Compile cefsimple and libcef_dll_wrapper
    * cefsimple is required for the multi-process model described above, and we use it as a bare-bones host of additional processes.
    * In c:/code/cg/chromium/src/cef/binary_distrib there should be a cef_binary_xxxxx_windows64 folder. If there's not, re-evaluate your life choices and the build steps above
	* Open a command window there, make sure you have cmake on your path, and run: `cmake -G "Visual Studio 15 2017 Win64"`
    * Open cef.sln in Visual Studio 2017
    * Replace the contents of cefsimple/cefsimple/cefsimple_win.cc wit the contents of ds_cinder/projects/web/cef/build/cefsimple_win.cc
    * Compile the whole solution in Debug and Release
* Replace files in ds_cinder/projects/web/cef/ from the binary_distrib folder from the previous step
    * include/ with everything from include/
    * lib64/debug/libcef.lib from Debug/libcef.lib
    * lib64/debug/libcef_dll_wrapper.lib from libcef_dll_wrapper/Debug/libcef_dll_wrapper.lib
    * lib64/release/libcef.lib from Release/libcef.lib
    * lib64/release/libcef_dll_wrapper.lib from libcef_dll_wrapper/Release/libcef_dll_wrapper.lib
    * lib64/runtime with everything from tests/cefsimple/Release/
* Open a ds_cinder sample project that has web playback (cef_develop perhaps?), clean, build
    * If there were significant changes in CEF, you'll need to evaulate those manually and resolve
    * Once compilation of the ds_cinder app succeeds and the app is running, do a few tests
        * Load https://www.whatismybrowser.com/ to verify the correct Chrome version from the branch you selected
        * Check that scrolling, touch, keyboard, reload, back, forward work
        * Check that webGL works with something from https://webglsamples.org/
        * Check that h.264 playback works by going to http://downstream.com/innovation_story/interactivity#play



