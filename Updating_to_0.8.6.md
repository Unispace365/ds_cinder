Updating to Cinder 0.8.6 and ds_cinder 0.8.6
===================================

* Grab Visual Studio 2013 (Express is fine, get the Desktop version)
* Grab Cinder 0.8.6 for visual studio 2013 and uncompress it: http://libcinder.org/download/
* Checkout the 086-develop branch of ds_cinder (at some point this will be the master branch)
* Create a CINDER_086 environment variable will now need to point to Cinder 0.8.6 (not ds_cinder, regular cinder)
* Create a DS_PLATFORM_086 environment variable and point it at ds_cinder. *Note:* this allows you to have side-by-side 0.8.4 and 0.8.6 cinder and ds_cinder installs. Simply clone a second ds_cinder repo and point DS_PLATFORM_086 at it.
* Add an environment variable for Gstreamer: DS_CINDER_GSTREAMER_1-0 that points to the base gstreamer install, e.g. c:/gstreamer/1.0/x86/

If you are updating an existing app, you'll need to:
------------------------

* Update all DS_PLATFORM references to DS_PLATFORM_086 in vcxproj, sln and property sheet files in your project
* Update all CINDER references to CINDER_086 in vcxproj, sln and property sheet files in your project
* Update all vc10 references to vs2013


Possible compilation issues:
-----------------------

* SerialRunnable: You may need to pass an alloc function when initializing
* Boost::mutex to std::mutex. In most cases for threading, the boost versions are supplanted with the std version. Check stack overflow / google, there's plenty of upgrad examples
* "Not defined": Many std elements now need to have include's explicitly defined, most commonly memory, cctype and sstring
* "KeyEvent Not Defined": Since the removal of using namespace ci::* from ds_cinder files, you'll need to make sure everything is namespaced properly. 
