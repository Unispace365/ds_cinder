Settings Files
========================


# What are they?

Settings files are xml files that ds_cinder apps load to configure the application. The primary setting file - engine.xml - sets basics for the app, like window size and position, frame rate, touch settings, and more. Most apps include other settings files to define fonts, colors and app-specific settings. 

## Editing

When running an app, press the "e" key to show the editor. You can browse through all the loaded settings by clicking the title. You can also save out the current settings to the common locations (see below). When editing a specific value, you can use your physical keyboard or the touch input to edit the value. Press escape to back out and use the keyboard normally in the app.

## Format

There are two pieces to any setting: the xml setting, and it's c++ counterpart. The xml setting looks something like this:

    <setting name="particular_component:speed" value="100.0" />
	
In c++ land, the setting gets ready like this:

    float theSpeed = mEngine.getAppSettings().getFloat("particular_component:speed");
	

## Indicies

You can have multiple instances of the same setting, to define a series of things:
    
    <setting name="component:address" value="127.0.0.1" />
    <setting name="component:address" value="8.8.8.8" />
	
In c++:

    std::string firstAddress = mEngine.getAppSettings().getString("component:address", 0);
    std::string secondAddres = mEngine.getAppSettings().getString("component:address", 1);

You can also iterate through each setting with the same name:

    int i = 0;
    while(true){
        std::string theAddress = mEngine.getAppSettings().getString("component:address", i);
        if(theAddress.empty()) break;
        i++
    }
	
If you want to look through all the settings in a settings file:

    mEngine.getSettings("FONTS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting){
        std::cout << "The value is " << theSetting.getString() << " for setting name " << theSetting.mName << std::endl;
    });
	
## Default Values

Pass a default value to the getter, and if the setting doesn't exist in the xml file, the default value will be returned.

    ci::vec2 viewerSize = mEngine.getAppSettings().getVec2("viewer:size", 0, ci::vec2(1920.0f, 1080.0f);
	
In the settings editor, you'll not only see any values set from xml, but called with a getter. You could define the entire settings file using c++ if you wanted. Check out the bottom of src/ds/app/engine/engine_settings.cpp to see how the engine.xml file is defined.

## Types

You can define the type of the setting as a hint to the editor for how to handle the setting. In XML:

    <setting name="debug:show" value="true" type="bool" comment="Show the debug stuff" />

In C++:

    getSetting("debug:show", 0, ds::cfg::SETTING_TYPE_BOOL, "Show the debug stuff", "false");

This doesn't prevent getting that setting by any other type, it just defines how the editor displays the setting. For instance, this would return "true" or "false" as a string:

    std::string showConsole = mGlobale.getAppSettings().getString("debug:show");
	
	
# Where are they located?

The main settings files are located in the settings/ folder. By default, this is the first place an app looks for settings files. 

## Configuration Overrides

 Here's where stuff gets fun. You can place settings files in folders inside the settings/ folder. These settings will be read after the main settings files, and overrite any values. For instance, you can have a main engine.xml that specifies a window to be 1920x1080, then you can have an override configuration that also has an engine.xml file that sets the window to 3840x2160. This can be used for development purposes, such as when you are building an app for many screens but are working on a laptop. You can also create multiple folders for different uses for your app. A single app could be built to run on a small single screen or on a video wall, and the configuration folders can be a useful tool to handle both cases.

 Look at the full_starter example project for how to setup configuration override folders. Note that any settings can be placed in the override file, not just window size.
 
## Local Overrides

Much like configuration overrides, you can also override settings by having additional settings files in your documents directory. These get placed in [current_user]/documents/downstream/settings/[project path]/ . Project path is defined in the main engine.xml file, and used to look up additional settings files. For instance, if your project path is "sample/kittens", you can place settings files in documents/downstream/settings/sample/kittens/ These files are read after the main files but before any configuration overrides. Using local overrides is convenient for production, where you want to set a particular machine to have some settings, but still replace the entire app folder reliably. 

This is also a convenient place to put settings that you'll only need during development (such as to turn the console on) that you don't want checked into a git repo.

## Local Configuration Overrides

This is the deep end of the rabbit hole. You can place configuration folders in the local override directory. These files are read last of all the settings files. 


