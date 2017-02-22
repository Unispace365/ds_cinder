Curl building
===================

At some point you may need to update curl or whatever. It's currently linking statically, which has some weird caveats on windows. Here's what to do.

    * Clone the curl repo: https://github.com/curl/curl
	* You may need to switch to a stable tag / branch / whatever. As of this writing, it's using the 7_42 branch
	* For each machine configuration (x86 and x64) do the following:
	    * Open a Microsoft Visual Studio command prompt for the relevant machine (x86 or x64)
	    * cd to the winbuild directory of the curl repo you cloned
	    * Run this command for release build: nmake /f Makefile.vc mode=static GEN_PDB=no DEBUG=no VC=12 machine=[MACHINE TYPE HERE GOOBER] RTLIBCFG=static
	    * Run this command for debug build: nmake /f Makefile.vc mode=static GEN_PDB=yes DEBUG=yes VC=12 machine=[MACHINE TYPE HERE GOOBER] RTLIBCFG=static
		* IMPORTANT: The RTLIBCFG=static is some magic that allows for static linking, otherwise you will have major trubs. If you build curl, then add that line, you'll need to delete the build intermediate files before building again
		* Copy the include files from one of the outputted builds above to ds_cinder_0.9.0\projects\essentials\src\ds\network\curl\
		* Copy the relevant .lib and .pdb files from the curl/builds/ folder to the relevant ds_cinder_0.9.0\projects\essentials\lib or lib64 folder. Use yer brain to figure out which files go where, dummy!