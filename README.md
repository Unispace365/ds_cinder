# DS Cinder

DS Cinder is a framework for interactive applications built on top of the fantastic Cinder framework https://libcinder.org/  DS Cinder provides conveniences for getting graphics onscreen; loading data from a sqlite database; displaying PDFs, videos and web pages; providing touch interaction; and loading settings. This thang is designed to make creative touch applications easy to get developed and into production. There's tons of pre-built stuff for creating buttons, keyboards, loading layouts, and more. 



## Getting Started

### Windows

**Install Visual Studio 2015**

1. Download here: https://www.visualstudio.com/vs/older-downloads/
2. Install it!

**Download Cinder 0.9.1**

1. https://libcinder.org/download 
2. Download Visual C++ 2013 edition
3. Unzip the downloaded folder
4. Make a new environment variable
    1. Open the Start menu and type "Advanced System Settings"
    2. Click "Environment Variables"
    3. Under "Sytem variables", select "New..."
    4. Variable name: CINDER_090
    5. Variable value: the path of the unzipped cinder 0.9.1 directory (e.g D:\my_code\cinder_0.9.1_vc2013)
    6. Ok, Ok, Ok
5. Open Visual Studio 2015 and open cinder.sln in cinder_0.9.1_vc2013/proj/vc2013
6. You should be prompted to update the compiler version. If not, right-click the cinder project and upgrade the compiler version
7. Go to the Build menu > Configuration manager. Select "Debug" for configuration and "x64" for platform at the top and close
8. Build using F7 or Build menu > Build solution
9. After the build completes, go back to step 7 and Select "Release" and "x64", then do step 8 again
10. You can close the Visual Studio window at this point

**Install Gstreamer (if you're planning to play videos)**

1. Go to https://gstreamer.freedesktop.org/data/pkg/windows/
2. As of this writing, the 1.12.4 version is what you want. Even-number minor versions (the "12" in this case) are the stable releases and the ones to use.
3. Download gstreamer-1.0-devel-x86_64-1.12.4.msi
4. Download gstreamer-1.0-x86_64-1.12.4.msi
5. Note: the version numbers will be different if you've selected something else. You want both the "devel" and regular installers, and the x86_64 versions
6. Install both msi installers with the "complete" option
7. If you have any Visual Studio windows open, close them now so they can get the new environment variables set by the gstreamer installers

**Download and compile DS Cinder**

1. Download and install git lfs (large file storage, one binary is too large for regular git) https://git-lfs.github.com/
2. Clone this repo with git
3. Run: git lfs fetch --all
4. Make another new environment variable
    1. Open the Start menu and type "Advanced System Settings"
    2. Click "Environment Variables"
    3. Under "Sytem variables", select "New..."
    4. Variable name: DS_PLATFORM_090
    5. Variable value: the path of this downloaded repo (e.g C:\Users\GordonN\Documents\ds_cinder)
    6. Ok, Ok, Ok
5. Open an example project in the example folder and compile
  
  
## Starting a project

There's a project cloner in utility/project_cloner.ps1. Right-click that file and Run with Powershell. That applet will duplicate the full_starter example project with a name and namespace of your choice. From there, you can customize the app as you see fit. The full starter example comes with most of the extention projects (web, pdf, video, etc) already added, so you can start developing right away without messing with property sheets or compilation settings. 

When getting to know DS Cinder, take a look at the docs/ directory, which has some background info on the basics. It's also recommended to look through the examples for how to use the various components.

### Linux + Mac

Linux is somewhat supported, but needs some work to be easy to use. Mac support is nearly there. 
