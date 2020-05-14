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
- Use the guide at the bottom of products/web/cef/readme.md


# MuPDF

- Download the source from here: https://mupdf.com/downloads/
- The download comes with win32 project files
- Open the sln in platform/win32/
- Change the output to debug (then release after that!) and x64 and compile
- Note that it's likely that a couple of the projects might fail to build, that's ok as long as libmupdf, libthirdparty and libfonts succeed


