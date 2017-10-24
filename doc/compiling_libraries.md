### Updating Compilation Stuff

# Cinder 

- Recompile in visual studio for Debug and Release in 64 bit
- Identify the output file (generally cinder.lib)


# Boost

- Identify which .lib files need updating
- Currently using 1.60.0  http://www.boost.org/users/history/
- Download and extract the source files
- Follow the build directions here: http://www.boost.org/doc/libs/1_62_0/more/getting_started/windows.html
- command line to build (the parameters are suber important): 
    bootstrap.bat
	.\b2 runtime-link=static architecture=x86 address-model=64
- If you need help, .\b2 --help
- Lib files will be in [boostdir]\stage\lib



# Poco

- Current version included is 1.7.9
- Download and extract Basic Edition source files from https://pocoproject.org/download/index.html
- Ignore the README linked on that page, and use the buildwin.cmd in the root directory
- Open the visual studio 2015 64 bit developer command prompt by opening a normal command window, then run this: "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"
- Using the dev 64 bit command prompt to build (cd to the root poco directory): buildwin.cmd 140 build static_mt both x64 nosamples notests
- Lib files will plop out into [pocodir]/lib64
- NOTE: if you're replacing the header files, then you may need to alter UnWindows.h, which undefines a bunch of windows macros 



# Snappy

- Grab the source code from https://github.com/google/snappy
- Run CMake:
    mkdir build
    cd build
	cmake ../
- Open the visual studio project that was created in the build folder
- Create a new configuration for 64 bit (configuration manager, clone from Win32)
- Open the properties for the 'snappy' project, and under C/C++ -> Code Generation change Runtime Library from Multi-threaded DLL to Multi-threaded (and debug if appropriate)
- Under Librarian, you may have to remove a flag for machine:x86 to compile in 64 bit
- Compile the 'snappy' project in debug and release, output is in [snappydir]/build/x64/[configuration]/snappy.lib


# Curl

- Clone the curl repo: https://github.com/curl/curl
- Check out a release tag. As of this writing, it's using the curl-7 branch
- Open a Microsoft Visual Studio command prompt by opening a normal command window, then run this: "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"
- cd to the winbuild directory of the curl repo you cloned
- Run this command for release build: nmake /f Makefile.vc mode=static GEN_PDB=no DEBUG=no VC=12 machine=[MACHINE TYPE HERE GOOBER] RTLIBCFG=static
- Run this command for debug build: nmake /f Makefile.vc mode=static GEN_PDB=yes DEBUG=yes VC=12 machine=[MACHINE TYPE HERE GOOBER] RTLIBCFG=static
- IMPORTANT: The RTLIBCFG=static is some magic that allows for static linking, otherwise you will have major trubs. If you build curl, then add that line, you'll need to delete the build intermediate files before building again
- You may get build errors, but as long as libcur_a.lib gets built, you're good to go
- Copy the include files from one of the outputted builds above to ds_cinder_0.9.0\projects\essentials\src\ds\network\curl\
- Copy the relevant .lib and .pdb files from the curl/builds/ folder to the relevant ds_cinder_0.9.0\projects\essentials\lib or lib64 folder. Use yer brain to figure out which files go where, dummy!


# CEF / Chromium Embedded Framework (aka web)

- Follow the guide here: https://bitbucket.org/chromiumembedded/cef/wiki/MasterBuildQuickStart
- When it comes up make the update.bat file, use this instead:
    set CEF_USE_GN=1
    set GN_DEFINES=is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome
    set GN_ARGUMENTS=--ide=vs2015 --sln=cef --filters=//cef/*
    python ..\automate\automate-git.py --download-dir=C:\code\chromium_git --depot-tools-dir=C:\code\depot_tools --no-distrib --no-build
	
- Use this for create.bat:
    set CEF_USE_GN=1
    set GN_DEFINES=is_win_fastlink=true proprietary_codecs=true ffmpeg_branding=Chrome
    set GN_ARGUMENTS=--ide=vs2015 --sln=cef --filters=//cef/*
    call cef_create_projects.bat
	
- The proprietary codecs value enables h.264 and mp3 support (which everyone uses these days pretty much)
- Compile both Debug_GN_x64 and Release_GN_x64
- After compilation succeeds, the relavent files are in C:\code\chromium_git\chromium\src\out\
- libcef.dll.lib is in the root directory of each configuration, and libcef_dll_wrapper.lib is in obj/cef/
- You'll need to update the headers in the cef_web project as well as update any relevant APIs
- Recompile cef simple, with everything removed except the CefExecuteProcess. cefsimple runs as a separate process for running browser instances. 
	* Open the cefsimple.vcxproj in Release_GN_x64 (it's in C:\code\chromium_git\chromium\src\out\Release_GN_x64\obj\cef)
	* Set the project subsystem in the linker settings to WINDOWS (if it's not already) This ensures that a console window doesn't pop up during runtime
	* Remove everything in cefsimple_win.cc and replace with this source code:

	#include <windows.h>
	#include <include/cef_app.h>
	int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
	){
		CefMainArgs main_args;
		return CefExecuteProcess(main_args, NULL, NULL);
	}
	* Recompile just cefsimple
- Move cefsimple.exe and all other lib, .h, .cc, .cpp, .dll files required from the new binary distribution to the cef_web project. Use the existing files as a guide.
- If all goes well, web sites should work fine and dandy and video playback should be working (use downstream.com's video sections to test)


# MuPDF

- Download the source from here: https://mupdf.com/downloads/
- The download comes with win32 project files
- Open the sln in platform/win32/
- Change the output to debug (then release after that!) and x64 and compile
- Note that it's likely that a couple of the projects might fail to build, that's ok as long as libmupdf, libthirdparty and libfonts succeed


