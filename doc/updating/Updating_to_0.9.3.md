# DS Cinder - Transitioning to Cinder 0.9.3 / C++17 / Visual Studio 1019

## text.xml + colors.xml => styles.xml
On the updated develop branch, the app now prefers styles.xml to the older pattern of a separate text.xml and
colors.xml. The easiest way to generate a matching sytle.xml for your existing project is to compile it with the updated
DsCinder and use the settings editor to save the generated styles.xml.

Once you've generated the styles.xml, delete the old text.xml and colors.xml from the settings and double check that the
app looks the same. 

If you don't generate a styles.xml your project will still work as before, however if you refresh with 'r' the fonts and
colors be messed up. Refresh again to get back to the correct configuration.

### Text
Applying text styles from C++ has also been updated:
```cpp
// Old style
ds::ui::Text* label = mEngine.getTextCfg("viewer:title").create(mEngine, mTrayHolder);

// New style
ds::ui::Text* label = new ds::ui::Text(mEngine);
label->setTextStyle("thing:label");
```

## New engine.xml settings
### screen:auto_size
screen:auto_size lets the application automatically adjust to the output display. This is particularly nice for
development and preview builds and avoids needing to create a custom config for each possible output.
```xml
<setting name="screen:auto_size"
         value="letterbox"
         type="string"
         comment="Classic uses the old src_rect/dst_rect and span_all_displays; letterbox will letterbox to your main monitor; all_span spans all displays; main_span fills the main display. letterbox requires having a world size set." 
         default="classic" possibles="classic, letterbox, all_span, main_span"/>
```

### downsync
```xml
<setting name="run_downsync_as_subprocess" value="true" type="bool" comment="Will Attempt to run downsync from the %APP%/downsync folder" default="true"/>
<setting name="downsync_config_file" value="%APP%/downsync/config.staging.json5" type="string" comment="Downsync with attempt to use this config when run as a subprocess" default="true"/>
```

## Yoga
... ?

## Other changes
### std::random_shuffle()
Our update to C++17 means that std::random_shuffle() needs to be replaced with std::shuffle(). The main thing that means
for you is that instead of it relying on the internal rand() function (which has global state), it now requires a
generator to be passed in. In the simplest form that looks like:
```cpp
std::random_device rd;
std::mt19937 generator(rd());

std::shuffle(things.begin(), things.end(), generator);
```
However, the [Mersenne Twister 19937](https://www.cplusplus.com/reference/random/mt19937/) algorithm has a fairly large
internal state so you should avoid creating a new generator for every shuffle operation. Instead, prefer using the
random_device and mt19937 as members.
