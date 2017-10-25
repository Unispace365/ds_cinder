### Updateing to 0.9.1 and vs2015


## Global Setup

1. Download cinder 0.9.1: (it'll say for 2013, that's ok): https://libcinder.org/download
2. Set the CINDER_090 environment variable to point to your newly-unzipped 0.9.1 directory
3. Open CINDER_090/proj/vc2013/cinder.sln in Visual Studio 2015
4. Compile cinder in Debug x64 and Release x64
5. Compile and run a ds_cinder example to make sure everything is hunky-dory


## Per project update

* Update the vs2013 folder to vs2015
	1. Delete any ipch, Debug, Release, or obj folders in the vs2013 folder
	2. Delete any .sdf files
	
* Edit the project sln file in a text editor
	1. Replace all 2013 references with 2015
	2. Change "Microsoft Visual Studio Solution File, Format Version 12.00" to 14.00
	3. Change "VisualStudioVersion = 12.0.40629.0" to 14.0.40629.0
	4. Save and close

* Edit vcxproj in a text editor
	1. Replace all 2013 with 2015
	2. Under the property sheets section, delete any references to property sheets for Win32 configurations. These files don't exist anymore, and visual studio won't let you open the project with broken links. In the next step we'll be deleting the 32bit configuration.
	3. The references to delete look like this: <Import Project="$(DS_PLATFORM_090)\vs2013\PropertySheets\Platform.props" />
	4. NOTE: keep any property sheet references for 64bit configurations
	5. Save and close

* Open SLN (should auto-launch vs2015)
	1. If you did the previous steps perfectly, it should ask to update the project compiler
	2. If the project load failed, try to reload it and resolve any issues. Generally it's because it's trying to link to a property sheet that doesn't exist. When you re-load the project, you'll need to Upgrade the compiler version by right-clicking on the project
	3. Under the Build menu, open the Configuration Manager
		* Click the value under "Active solution platform:" and select <Edit...>
		* Select Win32 then click Remove, Yes, then Close
		* Click the project's platform value (should be x64), then select <Edit...> and remove the Win32 platform there as well.
		* If you get an error trying to remove a platform, be sure the current platform is x64 before removing.
		* Close the configuration manager
	4. Clean and build
	5. If you encounter errors, generally restarting Visual studio and checking paths fixes the issues
	
* Gitignore, add these:
	1. *.db
	2. *.openDb
	3. GPUCache/