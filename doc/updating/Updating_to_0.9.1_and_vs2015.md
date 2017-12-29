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
	
## Various API changes

* Main app class: keyDown() is now onKeyDown(), and don't call the inherited class keyDown(), which will infinite loop. Some keys are now reserved by the app class. You can disable that by calling setAppKeysEnabled, which sends all keys to your app
* enableCommonKeystrokes() has been removed. 
* All mGlobals.getAppSettings(), or getLayoutSettings() need to be non-const
* Press "e" to edit engine settings
* Press "f" to toggle fullscreen
* All logger settings have moved to engine.xml, so if that's all that's in your debug.xml, you can delete debug.xml

## Update Settings Files

* Compile and run the settings_rewrite example. This app updates settings xml files to the new format.
* For most settings files, use the auto-detect method.
* Drag any settings xml files directly onto the app to update them
* If you're updating an engine.xml file but DON'T want the full engine settings, turn auto-update off. For instance, if you have a configuration override engine.xml with only a couple settings, turn auto-detect off before updating
* Recommend using this on git-committed files, in case you need to revert


## New Settings xml format

The settings xml format has been completely rewritten from previous versions. The new version is easier to expand on, more flexible, and allows for some helpful features. Using the in-app editor, you can see which xml file each setting originated in, as well as update the setting while running. You can also save out the current settings to the app directory or the override directory in local path. 

* mSettings.getText() has been replaced with mSettings.getString() or getWString()
* getSize() is now getVec2()
* getPoint() is now getVec3()
* getColor() and getColorA() now require a SpriteEngine reference (for named colors)

**Old settings format:**

    <text name="some:setting" value="the_actual_thing" />
    <text name="some:bool_setting" value="true" />
    <color name="some:color_setting" r="25" g="14" b="123" />
    <size name="some:vec2_setting" x="123" y="456" />
    <rect name="some:rectangle" l="123" t="0" r="456" b="789" />
	
**New settings format:**

    <setting name="A GUI-only Display Header" value="" type="section_header" />
    <setting name="some:setting" value="the actual thing" type="string" comment="This can explain what the setting does" />
    <setting name="some:bool_setting" value="true" type="bool" comment="Types are converted at runtime" />
    <setting name="some:color_setting" value="pink" type="color" comment="Colors can be names" />
    <setting name="some:color_setting" value="#ffff8c8c" type="color" comment="Or colors can be hex values" />
    <setting name="some:vec2" value="123, 456" type="vec2" comment="Vectors are comma separated with a space" />
    <setting name="some:vec3" value="123, 456, 789" type="vec3" comment="The space between values is important" />
    <setting name="some:rect" value="0, 0, 1920, 1080" type="vec3" comment="Rectangles are always L, T, W, H" />    
    <setting name="Number Values" value="" type="section_header" />
    <setting name="some:float" value="0.1" type="float" comment="Float, int, and double can have min/max values" min_value="0.0" max_value="1.0" />
    <setting name="some:double" value="0.1" type="double" comment="There are also default values, which are for convenience in the gui" min_value="0.0" max_value="1.0" default="0.1"/>
    <setting name="some:int" value="123" type="int" comment="int's are rounded to the nearest value" min_value="0.0" max_value="0" default="1000"/>
    <setting name="some:multi_select" value="choice A" type="string" comment="Seting a list of possible values for multi-select in the gui" possibles="choice A, choice B, choice C" default="choice A"/>
	
Note that since types are converted at runtime, there is no strict type-checking. So you could specify a value as an int in the xml, but still get the value as a wstring from the settings. The type markers are for documentation and for the gui to show the appropriate ui.

With the new system, every time you "get" a value, if the setting doesn't exist already, it'll be created and added to the settings. What you can do with this is specify all your settings in C++, run the app, then use the gui to save the file. Call getSetting() and fill out all the values:

    mGlobals.getAppSettings().getSetting("setting:name", 0, ds::cfg::SETTING_TYPE_STRING, 
    "Set the comment for the new setting, which is required", "default value", "min_value", "max_value", "possible_values");



* forEachTextKey() is now forEachSetting()

**Old syntax:**

    mEngine.getSettings("FONTS").forEachTextKey([this](const std::string& key){
        mEngine.editFonts().registerFont(ds::Environment::expand(mEngine.getSettings("FONTS").getText(key)), key);
    });

**New syntax:**

    mEngine.getSettings("FONTS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting){
        mEngine.editFonts().installFont(ds::Environment::expand(theSetting.mRawValue), theSetting.mName);
    }, ds::cfg::SETTING_TYPE_STRING);
	
	
**Old syntax:**

    mEngine.getSettings("COLORS").forEachColorAKey([this](const std::string& key){
        mEngine.editColors().install(mEngine.getSettings("COLORS").getColorA(key), key);
    });
	
**New syntax:**

    mEngine.getSettings("COLORS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting){
        mEngine.editColors().install(theSetting.getColorA(mEngine), theSetting.mName);
    }, ds::cfg::SETTING_TYPE_COLOR);
