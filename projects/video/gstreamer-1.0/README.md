Adding Gstreamer to your project
================================

* Download Gstreamer 1.14.4 from http://gstreamer.freedesktop.org/data/pkg/windows/1.14.4
* Get gstreamer-1.0-devel-x86_64-1.14.4.msi and gstreamer-1.0-x86_64-1.14.4.msi
* Install both with the "complete" option
* You should have gotten an environment variable for GSTREAMER_1_0_ROOT_x86_64 added for you. If not, add an environment variable for Gstreamer: GSTREAMER_1_0_ROOT_x86_64 that points to the base gstreamer install, e.g. c:/gstreamer/1.0/x86_64/
* Add the video visual studio project to your solution (make sure the sln uses environment variables DS_PLATFORM_093 to point to the video project)
* Add the property sheets using the property manager for both debug and release. Property sheets are in DS_PLATFORM_093/projects/video/gstreamer-1.0/PropertySheets.
* After you've added the project and property sheets, ensure that they are loaded relative to the DS_PLATFORM_093 variable by editing your solution and project files in a text editor.
* You may need to restart visual studio and/or your machine after adding environment variables, and maybe clean the solution as well.
* Create a Gstreamer video sprite and hit play on that suckah!
* Note that cloning a new project from the project cloner utility will come with the solution / project sheet additions noted above


Supported Formats and Resolutions
=================================

Gstreamer supports nearly every video format out there, and ds_cinder generally supports all video file types that gstreamer does. See below for file extensions.

Resolutions
-----------

* **1080p and below**
    * 1080p is generally defined as 1920x1080 pixels
	* ds_cinder supports almost all video types up to this resolution at 60fps.
	* This includes videos with transparent channels (QuickTime Animation and Apple ProRes 4444. Note that webm transparency isn't currently supported by gstreamer)
	* Recommended format
		* H.264 encoding, aka AVC
		* 5-10 mbps bitrate
		* 8 bit color in 4:2:0 profile
		* 30fps (24fps - 60fps ok)
		* Audio: AAC or mp3 at least 128kbps
		* High profile
		* VBR, 2-pass if available
		
* **4k and below**
	* 4k is generally defined as 3840x2160 pixels
	* ds_cinder plays videos of this size at 30fps with the following format
		* 3840x2160
		* H.264 encoding, aka AVC
		* 5-10 mbps bitrate
		* 8 bit color in 4:2:0 profile (4:2:0 is crucial)
		* 30fps (24fps - 30fps ok)
		* Audio: AAC or mp3 at least 128kbps
		* High profile
		* VBR, 2-pass if available
	* Videos with roughly the same number of pixels as 4k (for instance 4320 x 1920 pixels) will play ok
	
* **8k and below**
	* Currently, up to 8k videos are supported at 30fps on systems with newer NVidia GPUs, for instance a Quadro P4000
	* To use CUDA on an older video card, you'll be limited to 4096x4096 pixels
	* NVidia's CUDA decoder needs to be available in the GStreamer plugin directory (currently this plugin is in the gstreamer project folder: libgstnvdec.dll)
	* OpenGL mode and NvDecode mode need to be enabled on the video sprite or media player
	* The video needs to be h.265 (HEVC) or VP9. H.264 is not supported above 4k on CUDA.
	* Note that 10 and 12 bit color is not currently supported by the gstreamer nvdec plugin
	* Full details and support (see the grid for decoding on the bottom of the page): https://developer.nvidia.com/nvidia-video-codec-sdk
	* Recommended format
		* h.265 encoding aka HEVC
		* 8 bit color in 4:2:0 profile
		* 24-30fps
		* Audio: AAC or mp3 at least 128kbps
		* Width or height cannot exceed 8192 pixels for P-series cards and above (V, RTX), cannot exceed 4096 on older (M, K series)

		
File Extensions
---------------

ds::Resource, the data model that determines which type of media a file is, uses file extensions to determine what is or isn't a video. You can manually load a video from a filepath and avoid this. If you use a ds::Resource to load media these types are currently assumed to be videos:

mov
mp4
mp3
wav
avi
wmv
flv
m4v
mpg
mkv
3gp
webm
asf
dat
divx
dv
f4v
m2ts
mod
mpe
ogg
ogv
mpeg
mts
nsv
ogm
qt
tod
ts
vob
m4a
mxf
