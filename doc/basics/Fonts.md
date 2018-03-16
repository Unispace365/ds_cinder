# Fonts and Text Configs

## Using system-installed fonts

If a font is installed system-wide, or commonly comes with the OS (such as Arial), you can use that font by referencing that by name.

    mTextSprite->setFont("Arial");
	
Note that you need to refer to the exact font name. You can list all the installed fonts by pressing the 'p' key while an app is running. This is also how you can specify weights:

    mTextSprite->setFont("Arial Bold");
	
From a layout file:

    <text name="the_title"
		font_name="Arial"
		font_size="12"
		color="white"/>
	
## Custom fonts

Use the fonts.xml file to specify which font files to install when the app starts up. The font files will automatically be read and installed into the app's font system using the name of the setting as a font shorthand. You can define the values in fonts.xml like this:

    <setting name="noto_bold_file" value="%APP%/data/fonts/NotoSans-Bold.ttf" />

# Text Configs

Many apps define Text configs, which is a collection of font, color, line spacing and size. Text configs are generally set in text.xml and look like this:

    <text  name="default:error:name" value="Arial" />
    <float name="default:error:size" value="20" />
    <float name="default:error:leading" value="0.75" />
    <color name="default:error:color" code="dark_text" />
	
You can apply that to a text sprite in c++:

    mEngine.getTextCfg("default:error").configure(&mTextSprite);
	
Most commonly, Text configs are applied through layout files (see Dynamic Interfaces.md) using the "font" property:

    <text name="the_title"
		font="default:error"/>


## Shorthand Names

You can map font names to a user-defined shorthand name.

    mEngine.editFonts().registerFont("Arial", "title");
	
Now you can set Text sprite fonts by that shorthand:

    mTextSprite->setFont("title");
	
This is super convenient if you want to build a system where many fonts can be changed out at once or through a settings file.


	
## Manually loading fonts at runtime

If you don't want to use fonts.xml for whatever reason, you can install your own fonts using the something like the below:

    mEngine.editFonts().installFont(fullFontFilePath, fontName, shortHandName);
	
This is what ds_cinder is doing 'under the hood' at each app startup:

    mEngine.loadSettings("FONTS", "fonts.xml");
    mEngine.editFonts().clear();
    mEngine.getSettings("FONTS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting){
        mEngine.editFonts().installFont(ds::Environment::expand(theSetting.mRawValue), theSetting.mName);
    });
