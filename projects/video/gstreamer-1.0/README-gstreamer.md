Adding Gstreamer to your project
========================

* Download Gstreamer 1.2.3 from http://gstreamer.freedesktop.org/data/pkg/windows/1.2.3/
* Get gstreamer-1.0-devel-x86-1.2.3.msi and gstreamer-1.0-x86-1.2.3.msi
* Install both with the "complete" option
* Add an environment variable for Gstreamer: DS_CINDER_GSTREAMER_1-0 that points to the base gstreamer install, e.g. c:/gstreamer/1.0/x86/
* Add the video visual studio project to your solution (make sure the sln uses environment variables DS_PLATFORM_086 to point to the video project)
* Add the property sheets using the property manager for both debug and release. Property sheets are in DS_PLATFORM_086/projects/video/gstreamer-1.0/PropertySheets.
* After you've added the project and property sheets, ensure that they are loaded relative to the DS_PLATFORM_086 variable by editing your solution and project files in a text editor.
* Create a Gstreamer video sprite and hit play on that suckah!
