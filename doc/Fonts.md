
There are a variety of conveniences to make working with fonts easier.

1 LOGICAL FONT NAMES
----------------------
The first thing you should do is install logical names for all of the available fonts in your app constructor, something like:

      mEngine.editFonts().install(ds::Environment::getAppFolder("data", "fonts/DINPro-Black.otf"), FONT_DIN_BLACK);
      
where FONT_DIN_BLACK is just a defined name used to identify the font. The name you give the font will  be used both by the application and any settings files.

If you've done this, then you can just pass FONT_DIN_BLACK as the name to

      Text::setFont(const std::string& name, const float fontSize);
      


2 TEXT SETTINGS FILES
-----------------------
The second useful is to use the generic font settings format. See "example_settings/text.xml" for a complete file example. If you've got a file in this format, you can also install it in the app constructor, using this:

    mEngine.loadTextCfg("text.xml");
    
This will load the text.xml both from the app settings and any local settings. All settings created will be available from the sprite engine. For example, if text.xml has a font named "test", you can access it like this:

    SpriteEngine::getEngineCfg().getText("test");
