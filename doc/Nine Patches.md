Nine Patches
============

There are conveniences that make working with nine patches easier.


NINE PATCH SETTINGS FILES
----------------------
The second useful is to use the generic font settings format. See "example_settings/nine_patch.xml" for a complete file example. If you've got a file in this format, you can also install it in the app constructor, using this:

    mEngine.loadNinePatchCfg("nine_patch.xml");
	
This will load the nine_patch.xml both from the app settings and any local settings. All settings created will be available from the sprite engine. For example, if nine_patch.xml has a nine patch named "test", you can access it like this:

    SpriteEngine::getEngineCfg().getNinePatch("test");
