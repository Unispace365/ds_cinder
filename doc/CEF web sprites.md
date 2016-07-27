CEF Web Sprites
======================

Chromium Embedded Framework is a wrapper for Chromium, https://bitbucket.org/chromiumembedded/cef , which is the core of Chrome, a little-known web browser put out by a small California startup called "Google."  ds_cinder used to use Awesomium for web viewing, http://www.awesomium.com/ , which appears to be a dead project. Awesomium is also based on Chromium, but is closed-source and ancient. 

Ok, cool. Why do I care?
------------------------

CEF allows for much more control over the underlying browser and since the entire stack is open-source, the ability to fix any issues that come up. It's also up-to-date, meaning some small time websites, like google.com, actually function like they're supposed to and don't show security warnings. Additionally, more advanced functionality like that of Google Drive work properly.

Features
--------------

* Open source for maintainability
* Recent version of Chromium core, and ability to stay up-to-date in the future
* Functional HTML5 support
* More callbacks from the browser: fullscreen toggle, loading, title changes, errors, etc.
* Better performance with multi-process setup
* Security
* PDF viewing
* Customizeable error page
* Mouse / Touch / Keyboard input


Limitations
-------------

* WebGL is unsupported in the offscreen rendering that we need to use :(
* Proprietary media playback is also unsupported (like h.264 video). It's possible to recompile Chromium to support this, and this is in-progress


Still to-be-implemented in ds_cinder
-------------------------------------

* Find handling
* Drag-n-drop media (intra-browser dragging works)
* Geolocation should be working, but appears to be blocked somewhere
* Pop-up HTML select forms
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


Implementing into a project
============================

If your app already includes the "web" project for Awesomium-based web sprites, updating is easy. If your app solution doesn't include it already, the process is pretty much the same.

Including the project
---------------------

* Right-click on your solution, add an existing project, and select ds_cinder/projects/web/cef/cef_web.vcxproj
* Go to the Property Manager window, expand your project's selection
* Under "Debug | Win32", add ds_cinder\projects\web\cef\PropertySheets\Web_CEF_d.props
* Under "Release | Win32", add ds_cinder\projects\web\cef\PropertySheets\Web_CEF.props
* If you have Web_Awesomium property sheets included already, then delete them
* Save and close the solution
* Edit the solution and project files in a text editor, and update the absolute links above with environment variable links, for example:
    * In the solution file: "cef_web", "%DS_PLATFORM_086%\projects\web\cef\cef_web.vcxproj" 
	* In the project file: <Import Project="$(DS_PLATFORM_086)\projects\web\cef\PropertySheets\Web_CEF_d.props" />
	* This step is very important for others to be able to compile your project
* Compile and run. The relevant runtime files are automatically copied to the $Configuration folders (debug or release near your project file)

API Changes from Awesomium
---------------------------

The ds::ui::Web sprite API is nearly identical in both function calls and implementation. The main change is that you no longer need to "activate()" or "deactivate()" web sprites (which was unneeded to begin with).

Javascript is also currently not implemented. Feel free to do that then delete this comment! Thanks!


CEF Process and threading model
===============================

Be sure to check all the docs and comments thoroughly before making any changes.

Lifecycle and threading
-------------------------

* On startup, WebService initializes CEF. This creates a WebApp (manages the context) and WebHandler (deals with browser-to-sprite communication and requests)
* The ds_cinder main app thread creates a Web sprite.
* The WebService requests WebApp to create a new CefBrowser on the main thread. A browser is a single unit of web stuff, like a tab in Chrome.
* When the browser has been created asynchronously, it calls back to the main thread's Web sprite to notify it of the browser's identifier. 
* From there, the Web sprite registers callbacks for updated buffers, load state changes, title changes, etc. 
* Whenever the Web Sprite gets a callback or deals with any data shared with the browser, it must lock the main thread, as all callbacks from CEF come from CEF's UI thread. (CEF uses many threads, and the UI thread is the primary one)
* Whenever the Web Sprite makes a request of the browser (forwards, back, close, load, etc), the WebHandler must lock CEF, so updates to the same map/vector don't happen at the same time. 

Processes
-----------------------

CEF requires multiple processes for efficiently and securely loading web sites. Since the main ds_cinder process / executable handles making windows and consoles before any client code, we use a separate executable for the additional CEF processes. This executable is an extremely bare-bones CEF implementation based on SimpleApp, which is shipped with CEF binary distributions. cefsimple.exe is required to be in the working directory for this to function. Luckily, that exe is copied automatically by the cef_web build process. 

Resources
----------------------

CEF requires a number of resources to run. These are pak, bin, dll, dat and locale files. They're all included in ds_cinder/projects/web/cef/lin/runtime. Any updates to CEF should be sure to make sure that there are the proper resources in this folder. Everything in the runtime folder gets copied to the $Configuration folder at compile time.


