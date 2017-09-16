Settings update!
==================


Main changes and additions
--------------------------

* Main app class: keyDown() is now onKeyDown(), and don't call the inherited class keyDown(), which will infinite loop. Some keys are now reserved by the app class. You can disable that by calling setAppKeysEnabled, which sends all keys to your app
* enableCommonKeystrokes() has been removed. 
* All mGlobals.getAppSettings(), or getLayoutSettings() need to be non-const
* Press "e" to edit engine settings
* Press "f" to toggle fullscreen
* All logger settings have moved to engine.xml, so if that's all that's in your debug.xml, you can delete debug.xml


Update all your settings files using the settings_rewrite example
-----------------------------------------------------------------

* Compile and run the settings_rewrite example.
* For most settings files, use the auto-detect method
* If you're updating an engine.xml file but DON'T want the full engine settings, use the generic-only mode
* Recommend using this on git-committed files, in case you need to revert


New Settings xml format
-----------------------

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
	mGlobals.getAppSettings().getSetting("setting:name", 0, ds::cfg::SETTING_TYPE_STRING, "Set the comment for the new setting, which is required", "default value", "min_value", "max_value", "possible_values");



forEachTextKey() is now forEachSetting()

old syntax:
    mEngine.getSettings("FONTS").forEachTextKey([this](const std::string& key){
        mEngine.editFonts().registerFont(ds::Environment::expand(mEngine.getSettings("FONTS").getText(key)), key);
    });

new syntax:
    mEngine.getSettings("FONTS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting){
        mEngine.editFonts().installFont(ds::Environment::expand(theSetting.mRawValue), theSetting.mName);
    }, ds::cfg::SETTING_TYPE_STRING);
	
	
old syntax:
    mEngine.getSettings("COLORS").forEachColorAKey([this](const std::string& key){
        mEngine.editColors().install(mEngine.getSettings("COLORS").getColorA(key), key);
    });
	
new syntax:
    mEngine.getSettings("COLORS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting){
        mEngine.editColors().install(theSetting.getColorA(mEngine), theSetting.mName);
    }, ds::cfg::SETTING_TYPE_COLOR);