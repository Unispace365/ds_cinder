### Updating to a 64 bit build

This guide assumes you project already works with 0.9.0 in 32bit mode. If you haven't updated to 0.9 already, checkout the Updating_to_0.9.md file in this folder.


Overall Setup for your machine
------------------------------

You gotta have regular cinder compiled in 64 bit mode first. 

    * Open [CINDER_DIR]/vc2013/cinder.sln
    * Change the Platform setting (next to the Debug/Release configuration in the titlebar) to x64
	* Compile both debug and release
	* Close Visual Studio, cause who wants that open? Nobody.
	

For a specific project
----------------------

    * Open your solution file
	* In the title bar for Platform (it'll probably say Win32) open the drop down and select "Configuration Manager"
	* Under "Active solution platform", select <New...>
	* Under "Type or select the new platform" select x64, Copy settings from Win32, and make sure "Create new project platforms" is checked. Hit ok and close.
	* Go to the Property Manager pane
	* Twiddle open the Debug | x64 section
	    * Remove Platform_d
		* Add Platform64_d from [DS_PLATFORM_090]/vs2013/PropertySheets
		* Do the same for any other projects you've included. For instance, replace Essentials_d with [DS_PLATFORM_090]/projects/essentials/PropertySheets/Essentials64_d.props
		* Do the same 3 steps above for Release | x64, but use the versions without "_d"
		* Save and close the solution
		* Edit the project file and replace all the relative links with DS_PLATFORM_090 symbols.
		* For instance, replace this: ..\..\..\vs2013\PropertySheets\Platform64_d.props with $(DS_PLATFORM_090)\vs2013\PropertySheets\Platform64_d.props
		* Save
	* Open the project, and in properties for both Debug x64 and Release x64 change the Output Directory in the General Section to $(Configuration)\
	* That will make sure all the dll's get copied into the correct folder.
	* If you have included Curl locally in your project, add this define before you include the curl.h header:
	    #define CURL_STATICLIB
	* Compile and run the project

Common Issues / Troubleshooting
-------------------------------
	
	* Compile issues
		* Poco, Curl, Snappy, Freetype, and GTK (for pango) were all recently updated. Check any APIs you have that use those libraries and update
		* size_t to int warnings: Pretty common, cause in 64 bit land size_t is a 64 bit int, and int is still 32 bit. Be sure that this truncation won't cause any trubs.
		* Be sure you weren't storing any pointers as ints
	* Linking issues
		* If you see these, it's most likely that your app is linking to a 32bit lib. 
		* Try turning on verbose linking for searched libraries. 
		* Check that you've set the correct property sheets in the property manager for all configurations
		* Clean and rebuild
	* DLL errors / warnings on startup
		* Clean and rebuild
		* Make sure the output directory is $(Configuration)/
		* Check all dll's that they are in fact the 64bit versions
		* If you get an unhelpful 0000007b sorta message, delete all the dll's in your app directory and rebuild them