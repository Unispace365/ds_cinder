Every app gets its settings info from 1 or 2 settings files:

All apps are required to have an app settings file, the default settings that ships with a deployed app.

Optionally, an installation can have a local settings file.  The local settings are merged into the app settings, replacing or adding as necessary.

Where the settings are located and what the files are named will vary with the environment (development or production) and command line args.

1.  APP SETTINGS
The app settings file defaults to the name "engine.xml" and is located within the settings/ folder.  The settings/ folder is always relative to where the app .exe is located -- either at or within 3 levels above the folder containing the .exe (this is done solely to make development easier, so that the folder can be located several levels above the exe in the project structure).

The default name can be replaced by supplying a command line arg like this:  APP_SETTINGS=filename.xml

2.  LOCAL SETTINGS
The local settings get more complicated.  The simplest case is that the app settings contain a "project_path" setting, in which case, the local settings will be at the default location of Documents\settings\$(project_path\(app_settings_name.xml).  However, both the name and path can be changed via command line args:

LOCAL_SETTINGS=filename.xml
Using this will keep the project_path found in the app settings, but use a different file name.

LOCAL_PATH=$(project_path)\filename.xml
example:  LOCAL_PATH="northeastern\wall\engine_settings_ew.xml"
Using this will set the project path and local settings file name.  This completely replaces any project path specified in the app_settings, throughout the entire system.