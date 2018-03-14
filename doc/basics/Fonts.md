# Fonts and Text Configs

## Using system-installed fonts

If a font is installed system-wide, or commonly comes with the OS (such as Arial), you can use that font by referencing that by name.

    mTextSprite->setFont("Arial");
	
Note that you need to refer to the exact font name. You can list all the installed fonts by pressing the 'p' key while an app is running. This is also how you can specify weights:

    mTextSprite->setFont("Arial Bold");
	
## Loading fonts at runtime

Fonts can be loaded when the app starts up, allowing you to include fonts that don't need to be installed on each machine. At app startup:

    mEngine.editFonts().installFont(fullFontFilePath, fontName, shortHandName);
	
We'll get to shortHandName in a minute. 

It's most common that you'll want to load a few fonts all at once. Many apps do something like this:

    mEngine.loadSettings("FONTS", "fonts.xml");
    mEngine.editFonts().clear();
    mEngine.getSettings("FONTS").forEachSetting([this](const ds::cfg::Settings::Setting& theSetting){
        mEngine.editFonts().installFont(ds::Environment::expand(theSetting.mRawValue), theSetting.mName);
    });

You can define the values in fonts.xml like this:

    <setting name="noto_bold_file" value="%APP%/data/fonts/NotoSans-Bold.ttf" />

## Shorthand Names

You can map font names to a user-defined shorthand name.

    mEngine.editFonts().registerFont("Arial", "title");
	
Now you can set Text sprite fonts by that shorthand:

    mTextSprite->setFont("title");
	
This is super convenient if you want to build a system where many fonts can be changed out at once or through a settings file.

# Text Configs

Many apps define Text configs, which is a collection of font, color, line spacing and size. Text configs are generally set in text.xml and look like this:

    <text  name="default:error:name" value="title" />
    <float name="default:error:size" value="20" />
    <float name="default:error:leading" value="0.75" />
    <color name="default:error:color" code="dark_text" />
	
You can apply that to a text sprite like this:

    mGlobals.getText("default:error").configure(&mTextSprite);
	
Most commonly, Text configs are applied through layout files (see Dynamic Interfaces.md) using the "font" property.