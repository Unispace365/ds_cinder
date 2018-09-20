Basic XML interface
======================

```XML
    <interface>
        <sprite
            name="sprite_name"
            size="400, 200"
            position="100, 200, 300"
            color="#ff0000"
            enable="true"
            multitouch="all" >
            <text name="child_text" font="sample:config" resize_limit="300" />
            <image name="child_image" filename="RELATIVE PATH TO IMG" />
        </sprite>
    </interface>
```

Creating a Sprite and loading XML
-------------------------------
The tag value is the sprite type. For instance, <sprite/> creates a blank ds::ui::Sprite. Sprites are created as parents/children based on the XML hierarchy. Positions work relative to the parent (like normal).

Loading an interface XML to your class:

```cpp
    std::map<std::string, ds::ui::Sprite*>    spriteMap;
    ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layout/layout_view.xml"), spriteMap);
```

Adds the hierarchy in the interface as a child of "this".
Use the spriteMap to look up specific sprites by string name. This lets you apply a data model or deal with touch callbacks, etc. Avoid setting the layout in c++ land, as that can confuse where the layout is managed.

You can also preload the xml by keeping a ds::ui::XmlImporter::XmlPreloadData generated from ds::ui::XmlImporter::preloadXml() and pass that as the second parameter of loadXMLto(). This avoids a run-time hit to the disk, though all the parameter mapping still happens in real time.

Parameter Types
------------------------------
* **Vector**: Always 3d, but the second 2 parameters are optional. Vectors are comma delimited, with a space after the comma. Example: position="x, y, z", so position="100, 200, 300". Also valid: position="100" (which map to ci::Vec3f(100.0f, 0.0f, 0.0f)) or position="100, 200" (which is ci::Vec3f(100.0f, 200.0f, 0.0f))
* **Color**: Format: #AARRGGBB or #RRGGBB, or a named color specified by a call to mEngine.getColors().install().
* **Boolean**: These values will map to true: true, TRUE, yes, YES, on, ON. Everything else is false.
* **Blend**: Valid values: normal, multiply, screen, add, subtract, lighten, darken. Default = normal.
* **Float**: Specify the number with the decimal. The trailing "f" is unneccasary. For instance, opacity="0.5" not opacity="0.5f"

Sprite Types
--------------------------
* **sprite** = ds::ui::Sprite
* **image** = ds::ui::Image
* **image_with_thumbnail** = ds::ui::ImageWithThumbnail
* **image_button** = ds::ui::ImageButton
* **sprite_button** = ds::ui::SpriteButton
* **layout_button** = ds::ui::LayoutButton
* **text** = ds::ui::Text
* **gradient** = ds::ui::GradientSprite
* **layout** = ds::ui::LayoutSprite
* **persp_layout** = ds::ui::PerspectiveLayout
* **circle** = ds::ui::Circle
* **border** = ds::ui::Border
* **dashed_line** = ds::ui::DashedLine
* **donut_arc** = ds::ui::DonutArc
* **scroll_list[_(vertical|horizontal)** = ds::ui::ScrollList
* **scroll_area** = ds::ui::ScrollArea
* **smart_scroll_list** = ds::ui::SmartScrollList
* **centered_scroll_area** = ds::ui::CenteredScrollArea
* **control_check_box** = ds::ui::ControlCheckBox
* **control_slider[_(horizontal|vertical)]** = ds::ui::ControlSlider
* **scroll_bar** = ds::ui::ScrollBar
* **entry_field** = ds::ui::EntryField
* **soft_keyboard** = ds::ui::SoftKeyboard
* **web** = ds::ui::Web (if the cef web project is included)
* **pdf** = ds::ui::Pdf (if the pdf project is included)
* **video** = ds::ui::GstVideo (if the gstreamer-1.0 project is included)
* **media_player** = ds::ui::MediaPlayer (if the viewers project is included)
* **xml** = Load another xml interface. See details below.
* **[custom]** = Calls a custom callback function with a string for the type. Requires you instantiate the sprite type yourself.

Sprite Parameters
-----------------------

* **width**: Specify a float for the width of the sprite. width="100"
* **height**: Specify a float for the height of the sprite. height="100"
* **depth**: Specify a float for the depth of the sprite. depth="100"
* **size**: Specify a 2d vector for the width and height (depth not supported). size="400, 100"
* **color**: Sets the color of the sprite and makes it non-transparent (so it renders). Format: #AARRGGBB or #RRGGBB, AARRGGBB, or a named color specified by a call to mEngine.getColors().install(). For instance: color="#FF0033bb" or color="black".
* **opacity**: The opacity as a float from 0.0 to 1.0. Overrides any alpha specified in color. opacity="0.5"
* **position**: Position in pixels. position="100, 200, 300" or position="0, 50"
* **rotation**: 3d vector of the degrees (not radians) of rotation in x, y, z. rotation="100, 200, 300" or rotation="0, 0, 90"
* **scale**: 3d vector of the scale in x, y, z. Scale is from 0.0 (nothing) to 1.0 (100%), not bounded. scale="1, 1, 1" or scale="0.5, 0.5, 1"
* **center**: Center is where the anchor of the sprite is calculated from (for scaling and rotation) and is a percentage from 0.0 (top/left) to 1.0 (bottom/right), not bounded. center="0, 0.5, 1"
* **clipping**: Boolean of whether to clip it's children.
* **blend_mode**: Valid values: normal, multiply, screen, add, subtract, lighten, darken. Default = normal.
* **enable**: Boolean of whether to handle touch input or not.
* **multitouch**: String of multitouch mode. Possible values:
    * info = MULTITOUCH_INFO_ONLY
    * all = MULTITOUCH_NO_CONSTRAINTS
    * pos = MULTITOUCH_CAN_POSITION
    * scale = MULTITOUCH_CAN_SCALE
    * pos_x = MULTITOUCH_CAN_POSITION_X
    * pos_y = MULTITOUCH_CAN_POSITION_Y
    * pos_scale = MULTITOUCH_CAN_POSITION | MULTITOUCH_CAN_SCALE
    * pos_rotate = MULTITOUCH_CAN_POSITION | MULTITOUCH_CAN_ROTATE
    * rotate = MULTITOUCH_CAN_ROTATE
* **transparent**: A boolean of wheather the sprite should draw or not. This works only locally. For base sprite, this will draw a rectangle (if the sprite has a size)
* **visible**: A boolean of the visibilty flag. Doesn't affect the draw status, but does turn off this sprite and any children if hidden. True = show(), false = hide()
* **animate_on**: Supply a script to run when tweenAnimateOn() is called on this sprite. See the animation section for details.
* **corner_radius**: A float the changes the corner radius. Only applies to some sprite types like Sprite and Border. Many types ignore this setting. Default=0.0.
* **on_tap_event** and **on_click_event**: Dispatches one or more events from tap or button click. on_tap_event uses the built-in tap callback for any sprite, on_click_event only applies to ImageButton and SpriteButton. See the events section for more details.
* **model**: Apply ContentModelRef properties to sprite properties. See the Content Model section below.


Variables
----------------------

Add dynamic variables to any sprite parameter. Variables are pulled from app_settings.xml automagically. In addition, there are a few default variables for common engine properties. Specify a variable in an xml layout by pre-pending it with "$_".

```xml
<interface>
	<layout name="example"
		size="$_world_size"
		pad_all="$_padding"
		animate_on="$_default:anim"
		/>
</interface>
```

Variables are replaced first before expressions (see below), and parsed recursively. For example, in app_settings.xml you can specify a variable that references another variable:

```xml
	<setting name="default:delay" value="0.1" />
	<setting name="default:anim" value="fade; slide:$_world_width, 0; ease:outQuint; delay:$_default:delay" />
```

* Add variables in c++ using ds::ui::XmlImporter::addVariable()
* Variables are all treated as strings, so you can replace any sprite property, such as font name, animation script, alignment, etc
* If a variable replacement isn't found, the original value is used and a warning is logged

### Default Variables

* **world_width**: The equivalent of mEngine.getWorldWidth()
* **world_height**: The equivalent of mEngine.getWorldHeight()
* **world_size**: The equivalent of "mEngine.getWorldWidth(), mEngine.getWorldHeight()"
* **anim_dur**: The equivalent of mEngine.getAnimDur()


Expressions
--------------------

Layouts support math expressions that are parsed when the layout is loaded. Expressions are parsed after variables, and before properties are assigned. There are two ways to indicate that a value is an expression: By starting a line with #expr or by wrapping an expression in #expr{}.

```xml
<interface>
	<layout name="example"
		pad_all="#expr 123.45 + 67.89 - sin(pi)"
		size="#expr{50.0 * 2.0}, #expr{$_world_height / 2.0}"
		/>
</interface>
```

Syntax Rules:

* Expression parsing syntax: http://warp.povusers.org/FunctionParser/fparser.html#usage
* Note that we don't support expression variables (such as sqrt(x * x + y * y)), the expression must evaluate to a single value
* The starting syntax must be "#expr{" with no spaces
* If you start a line with #expr (with no brackets), the entire rest of the line is evaluated as an expression
* pi is automatically defined


Events
--------------------------------------------------------

Using the **on_tap_event** and **on_click_event** sprite parameters, you can trigger events for other parts of the app directly from a layout xml. You'll need to do a couple things to get this to work.

* **Register events by name:** Create a ds::RegisteredEvent like normal by extending the RegisteredEvent class. You'll need to provide a creation function to the event registry so the event can be created dynamically when called. To do this, add a line like the below on app instantiation (in your root app class) for each event.

        ds::event::Registry::get().addEventCreator(RequestCloseAllEvent::NAME(), [this]()->ds::Event*{return new RequestCloseAllEvent(); });

* **Built-in event parameters:** The event will automatically have the sprite that triggered the event applied to the ds::Event::mSpriteOriginator property, and mEventOrigin paramter will be the global position of the tap or click of the interaction.
* **Custom event parameters:** You can apply certain parameters to each event dispatched from a layout xml: Data, Id, and UserSize. Pass these properties to the event like so:

        RequestCustomEvent; data:myCustomStringData; id:1234; user_size:400, 300, 1;

* **Multiple events:** Send multiple events from the same button press by wrapping each event in brackets and separating them by commas. Do not use spaces between events. Example:

        on_tap_event="{RequestCloseAllEvent},{RequestMediaOpenEvent; data:%APP%/data/temp/test.pdf; user_size:900},{RequestLayoutEvent}"

* **Handling events:** The app will need to handle the events like normal using an event client and handling the app event. The parameters described above are automatically applied to the Event by the xml importer and you can access them through the event:

```cpp
        void ViewerController::onAppEvent(const ds::Event& in_e){
            if(in_e.mWhat == RequestMediaOpenEvent::WHAT()){
                std::string fileName = in_e.mUserStringData;
                //Do something with the filename and open media
            }
        }
```
	

Layout Parameters : valid for any sprite if the parent is a layout
------------------------------------------------------
* **t_pad**: Padding on the top part of this sprite in the layout
* **b_pad**: Padding on the bottom part of this sprite in the layout
* **l_pad**: Padding on the left part of this sprite in the layout
* **r_pad**: Padding on the right part of this sprite in the layout
* **padding**: Set padding for all sides of sprite (order: L, T, R, B) ex: padding="20, 30, 20, 30"
* **pad_all**: Set padding for all sides of sprite together. ex: pad_all="60"
* **layout_fudge**: An offset in pixels for the sprite during the layout. Doesn't get added to the overall layout positioning, but allows you to fudge the position a bit if something isn't rendering just right
* **layout_size**: Desired size for the "fixed" layout size mode. This is separate from the sprite size, since some sprites (like Image) will be able to calculate size only after the thing is loaded, and this will try to scale the image (or other thing) up to fit in this size, letterboxed.
* **layout_size_mode**: The method to calculate the size of this sprite during layout
    1. fixed: (Default) The size of this sprite is not modified during the layout
    2. flex: Sprite will be made wide enough (for V layouts) or tall enough (for H layouts) during layout. Images will be sized proportionally to fit the size. Text fields in Vertical Layouts will be resized to the width of the layout (minus padding), and the height calculated (plus padding). Text fields in Horizontal layouts will be resized to their existing resize width limit and constrained to the height of the layout (including padding).
    3. stretch: Sprite and all stretch siblings will be sized to share the remaining space available in the layout, separated by spacing, and respecting padding. Stretch sprites will be given no space at all if the layout shrinks to its children along its layout axis.
    4. fill: Sprite will be moved to the parent layout origin and set to the size of the layout, respecting padding.
* **layout_v_align**: For sprites added as a child of a horizontal layout, the sprite will be aligned appropriately.
    1. top: (Default) Aligns the sprite to the top of the layout.
    2. middle: Vertically centers the sprite in the layout (only works on Fixed size sprites)
    3. bottom: Aligns the sprite to the bottom of the layout (only works on Fixed size sprites)
* **layout_h_align**: For sprites added as a child of a vertical layout, the sprite will be aligned appropriately.
    1. left: (Default) Aligns the sprite to the left of the layout.
    2. center: Horizontally centers the sprite in the layout (only works on Fixed size sprites)
    3. right: Aligns the sprite to the right of the layout (only works on Fixed size sprites)
* **layout_fixed_aspect**: Tells the sprite's parent layout if this sprite should be resized proportionally or not. Some sprites are fixed aspect ratio by default: Image, PDF, Video, ImageButton, Circle. This parameter is used for layout_size_mode of Flex, Stretch and Fill. If layout_fixed_aspect is true, the sprite will be fit inside the destination area, with letterboxing (unless it's a stretch size mode in a SizeType layout, then it won't letterbox). For layout_fixed_aspect to work, the sprite needs to have w & h != 0.0.
* **layout_type**: For a LayoutSprite only (has no effect on children).
    1. vert: (Default) lays out all children in a vertical (top to bottom) flow.
    2. horiz: Lays out children horizontally (left to right)
    3. size: Only adjusts the size of children, but does not modify their position. Useful to constrain some elements to a size, but keep them at a particular position
    4. <any other value>: None, does no layout of children.
* **layout_spacing**: For LayoutSprite only, a float that sets the spacing between all elements. layout_spacing="10.0"
* **shrink_to_children**: For LayoutSprite's only, determines how the sprite adjusts to its children.
    1. none, false, or no value: None, will not alter the size of this layout sprite after laying out the children
    2. width: Adjusts the width of this sprite to its children (for vertical, the widest child, for horiz, the total width of the children, plus spacing)
    3. height: Adjusts the height of this sprite to its children (for vertical, the total height of the children, for horiz, the tallest child)
    4. both or true: Both width and height
* **overall_alignment**: For LayoutSprite, allows you to align the contents to Top, Left, Center, Right, Middle or Bottom inside the layout

Perspective Layout Parameters
------------------------------------------------------------
* **persp_fov**: Float field of view in degrees. Default is 30
* **persp_auto_clip**: Bool enable or disable automatically setting the near / far clip distance
* **persp_auto_clip_range**: Float for how far around the eye distance to calculate the clip distance if auto clip is on
* **persp_near_clip**: Float for the near clipping distance, only if auto clip is disabled
* **persp_far_clip**: Float for the far clipping distance, only if auto clip is disabled
* **persp_enabled**: Bool to enable or disable rendering in 3d at all

Text Parameters : text
------------------------------------------------------------
* **text**: Set the content show on the screen. text="Hello World"
* **text_update**: Set the text content show on the screen, but only if there is content. See ContentModel section below. text="" // does nothing, text="Hey" // set's the text
* **text_uppercase**: Set the content show on the screen. text_uppercase="Hello World" shows up as HELLO WORLD
* **text_lowercase**: Set the content show on the screen. text_uppercase="Hello World" shows up as hello world
* **markdown**: Parses the string into markdown then applies it as text. markdown="Hello World, but including **markdown**"
* **font**: The text config. Set in settings/text.xml. The text config sets the font name, size, leading and color. font="sample:config"
* **font_name**: The name of the font registered in the app. **Note:** It's recommended you use the font setting above (a whole config) OR font_name and font_size, and not mix the two.
* **font_size**: Replace the original font size of Text sprites. font_size="20"
* **font_leading**: The multiplier of font_size to use for line height (only when text consists of multiple lines)
* **font_letter_spacing**: Set the letter spacing in pts (0.0 = normal, 1.0 = 1pt extra letter spacing, etc.)
* **resize_limit**: If you set only the width value, the layout will continue for all the text in the sprite, and the height will be calculated from that. resize_limit="400" or resize_limit="400, 500"
* **text_align**: Set the alignment of the Text Sprite
    1. "left": The default, normal text
    2. "center": Center-aligns the layout.
    3. "right": Align rows of text to the right side of the resize_limit

Image Parameters : image
-------------------------
* **filename** OR **src**: File path RELATIVE to XML. For instance: src="../data/images/refresh_btn.png"
* **filename_cache** OR **src_cache**: Exactly the same as above, but includes the ds::ui::Image::IMG_CACHE_F flag, which permanently caches the image
* **circle_crop**: Boolean. If true, will crop image content outside of an ellipse centered within the bounding box.
* **auto_circle_crop**: Boolean. If true, centers the crop in the middle of the image and always makes it a circle, persists through image file changes.

Image Button Parameters : image_button
-------------------------
* **filename**: Will apply the same image to down and up state. Will override down_image and up_image, and vice-versa.
* **down_image**: The image to display when touch is happening on this button. See filename for explanation. down_image="../data/images/refresh_btn.png"
* **up_image**: The image to display when touch is happening on this button. See filename for explanation. up_image="../data/images/refresh_btn.png"
* **btn_touch_padding**: How much space to add to the sprite on every side of the image. btn_touch_padding="40"
* **down_image_color**: Applies a color to the down image, to make creating responsive buttons easy from a single image.
* **up_image_color**: Applies a color to the up (normal) image, to make creating responsive buttons easy from a single image.


Sprite Button and Layout Button Parameters : sprite_button, layout_button
----------------------------
**attach_state**: Add this to a sprite that's a child of a sprite button or a layout button. Valid values: "normal" for the unpressed state and "high" for the pressed state. For example:

```cpp
    <interface>
        <sprite_button
            name="sample_button"
            size="400, 200" >
            <text  name="child_text" font="sample:config" text="The Button" />
            <image name="child_image" filename="%APP%/data/images/icons/up_icon.png" attach_state="normal" />
            <image name="child_image" filename="%APP%/data/images/icons/down_icon.png" attach_state="high" />
        </sprite>
    </interface>
```

Gradient Sprite Parameters : gradient
---------------------------
* **colorTop**: A color value to set the TL and TR colors of the gradient to. colorTop="ffffff"
* **colorBot**: A color value to set the BL and BR colors of the gradient to. colorBot="ffffff"
* **colorLeft**: A color value to set the TL and BL colors of the gradient to. colorLeft="ffffff"
* **colorRight**: A color value to set the TR and BR colors of the gradient to. colorRight="ffffff"
* **gradientColors**: Set all four colors for the gradient, specified clockwise from TL "[colorTL], [colorTR], [colorBR], [colorBL]". Example: gradientColors="#ff0000, #000000, #00ff00, #0000ff"

Circle Sprite Parameters : circle
---------------------------
* **filled**: Boolean, whether to draw just the outline or fill in the circle
* **radius**: Float, the radius of the circle to draw
* **line_width**: Float, the width for non-filled circle border

Border Parameters : border
---------------------------
* **border_width**: Float, the width of the border, which has its outer edge at the extent of the sprite

Donut Arc Parameters : donut_arc
---------------------------
* **donut_width**: Float, the distance in pixels from the outside of the sprite to the inside of the donut
* **donut_percent**: Float, the percentage that the donut is filled in

Dashed Line Parameters : dashed_line
---------------------------
* **dash_length**: The length of each dash in a dashed line
* **dash_space_inc**: The distance between each dash in a dashed line

Control Check Box Parameters : control_check_box
---------------------------
* **font**: Sets the font text config of the label (which also adds a true/false label at all, optional)
* **check_box_true_label**: Sets the text string that shows up when the box is checked
* **check_box_false_label**: Sets the text string that shows up when the box is unchecked

Scroll List Parameters : scroll_list
-------------------------------
* **Note:** You'll need to supply the usual callbacks for this to work (for creating items in the list, setting data, etc)
* **scroll_list_layout**: Sets the parameters for layout from the format "x, y, z", which translates to setLayoutParams(xStart, yStart, incrementAmount, true);
* **scroll_list_animate**: Sets the animation parameters, from the format "x, y", where x==startDelay and y==deltaDelay on ScrollList::setAnimateOnParams(startDelay, deltaDelay);
* **scroll_area_vert**: Sets the direction parameters, where true==vertical and false==horizontal on ScrollArea::setVertical(bool); **Note: only applicable to ScrollArea, not ScrollList. To set horizontality of ScrollList, use Sprite-types of 'scroll_list_vertical' and 'scroll_list_horizontal'.**
* **scroll_fade_colors**: **Also applicable to ScrollArea**. Set the colors of the scroll area, in the format "[colorFull], [colorTransparent]". Example: scroll_fade_colors="ff000000, 00000000" or scroll_fade_colors="44000000, 000000"
* **scroll_fade_size**: Set the size of the fade as a float.

SmartScrollList : smart_scroll_list
--------------------------------------
* **smart_scroll_item_layout**: Sets the layout file for each list item, relative to %APP%/data/layouts/

EntryField and SoftKeyboard Parameters : entry_field, soft_keyboard
--------------------------------------
EntryFields and SoftKeyboards need some parameters set for instantiation, so they are set as in the body of the node rather than as attributes. Example:

```xml
    <entry_field
        name="search_field"
        sprite_link="primary_keyboard"
        >text_config:keyboard:key:up; cursor_offset:4, -10; cursor_size:1, 40; field_size:500, 40; cursor_color:orange</entry_field>

    <soft_keyboard name="primary_keyboard">
    type:lowercase; key_scale:1; key_up_color:bright_grey; key_down_color:orange; key_text_offset:-2, -2; key_touch_padding:4</soft_keyboard>
```

**ENTRY FIELD PARAMETERS**
* **sprite_link**: Allows you to link the text entry field with a soft keyboard. Set the value of sprite_link to the name of the soft_keyboard. The keyboard needs to be in the same sprite map as entry field to be linked. Once linked, the keyboard will type it's text into the entry field.
* **text_config**: The text config for the text of the entry field. Default: entry_field:text
* **cursor_size**: How big the blinking cursor is in pixels. Default: 2, 36
* **field_size**: The size of the entry field, which also sets the resize limit of the text in the field. You can also set the size of this sprite to change the field size. Default: 500, 100
* **cursor_offset**: X/Y Pixels to offset the cursor from the end of the text sprite. For fudging the position of the cursor. Default: 2, -5
* **cursor_color**: The color of the blinking cursor. Engine colors are allowed. Default: white
* **blink_rate**: How many seconds to wait between blinks. Total blink time is animate_rate + animate_rate + blink_rate. Default: 0.5
* **animate_rate**: How many seconds to fade the cursor on and off. Default: 0.3.
* **text_offset**: How many pixels to offset the text sprite. Default: 0.0, 0.0
* **password_mode**: If true, will show bullets instead of text. Default: false

**SOFT KEYBOARD PARAMETERS**
* **type**: Determines which kind of keyboard this is. Valid types: standard, lowercase, extended, pinpad and pincode. Standard has shift abilities and some extended keys. Lowercase is simplified and only has lowercase keys. Pinpad is like an ATM pin pad with an enter button. Pincode is a number entry keyboard with a back/delete button. Default: standard
* **key_up_text_config**: The text config of the text in the keys when not pressed. Default: keyboard:key:up
* **key_dn_text_config**: The text config of the text in the keys when pressed. Default: keyboard:key:down
* **key_up_color**: The color of the keys when not pressed. Engine colors allowed. Default: white
* **key_down_color**: The color of the keys when pressed. Engine colors allowed. Default: medium grey
* **key_text_offset**: Vector, amount to fudge the offset of the text inside each key. Default: -5.0, -5.0
* **key_touch_padding**: Float amount between each key in pixels. Default: 4.0
* **key_initial_position**: Vector x/y of the start position of the first key when creating the keys. Default: 0, 0
* **key_scale**: The amount the keyboard is scaled as a single float. Default: 1.0
* See soft_keyboard_settings.h for the default text configs and key images.

Web Parameters : web
-------------------------------
If you have the web project included, you can create web sprites.
* **web_url**: The full url of a site to load.

PDF Parameters : pdf
-------------------------------
If you have the pdf project included, you can create pdf sprites.
* **pdf_src**: Relative or absolute path to the pdf. For example: pdf_src="%APP%/data/test/test.pdf" or pdf_src="c:/test.pdf"

Video Parameters : video
-------------------------------
If you have the video project included, you can create video sprites.
* **video_src**: Relative or absolute path to the video. For example: video_src="%APP%/data/test/test.mp4" or video_src="c:/test.mp4"
* **stream_src**: A pipeline for a live stream source or the URI of a stream. Example: stream_src="rtsp://192.168.1.37:5015/Stream1"

Media Players Parameters : media_player
-------------------------------
If you have the viewers project included, you can create media players. Media players are a simple way to view Images, PDFs, Videos, and Web sites. Media Players created this way automatically have an embedded interface (to flip through pages or control videos). Media types are deduced by file extension. MediaPlayer sprites need to be enabled or accept some user input for the interface to re-appear.
* **media_player_src**: Relative or absolute path to the media. For example:

        media_player_src="%APP%/data/test/test.mp4" or media_player_src="c:/test.pdf"
* **media_player_auto_start**: Boolean, if true, videos play automatically. If false, they'll play the first frame then stop
* **media_player_show_interface**: Boolean, true shows interfaces for pdf, web and video immediately
* **media_player_web_size**: Vector, sets the w/h in pixels of web views
* **media_player_video_volume**: Float, sets the volume of videos when they start
* **media_player_video_loop**: Boolean, true, the default, loops the video, false will play the video once and stop
* **media_player_video_reset_on_complete**: Boolean, true, the default, resets the video to 0.0 when the video finishes in non-loop mode
* **media_player_letterbox**: Boolean, true, the default, will letterbox the media inside the size of the media player, false fills (with no cropping by default). Web always fills
* **media_player_standard_click**: Boolean, default is false, true will allow tapping to start/stop videos, advance pdf's, and click into web pages. 

XML : xml
-------------------------------
You can load another xml interface from within an xml interface. This is super handy for menus and such that have a bunch of identical buttons or to make a consistent close button for your whole app.
Limitations:
* You cannot load the same interface recursively, it'd be infinite
* You can't add children to the loaded interface from the parent interface, since the child interface might have many children

Properties:
* You can set the properties of any loaded child from the parent with a "property" tag.
* Children are given a dot naming scheme, and calling properties uses names relative to the child's interface.

Example:

**menu_view.xml:**

```xml
    <layout name="layout" >
        <xml name="home" src="%APP%/data/layouts/menu_button.xml" >
            <!-- note that the "name" here refers to the name of a sprite inside menu_button.xml.
                Any other property of the child can be modified here as well -->
            <property name="normal_icon" src="%APP%/data/images/icons/home_up.png" />
            <property name="high_icon" src="%APP%/data/images/icons/home_down.png" />
        </xml>
       <xml name="map" src="%APP%/data/layouts/menu_button.xml" >
            <!-- you can't have any normal children here,
                 only set properties of the children of the parent xml interface -->
            <property name="normal_icon" src="%APP%/data/images/icons/map_up.png" />
            <property name="high_icon" src="%APP%/data/images/icons/map_down.png" />
            <property name="down_gradient" opacity="0.5" animate_on="fade" />
        </xml>
    </layout>
```

**menu_button.xml:**

```xml
    <sprite_button name="the_button" size="80, 80">
        <gradient name="down_gradient" attach_state="high" size="80, 80"
                  gradientColors="red_orange, red_orange, red_orange, orange"/>
        <image attach_state="normal" name="normal_icon"/>
        <image attach_state="high" name="high_icon" />
    </sprite_button>
```

**In c++:**

```cpp
    std::map<std::string, ds::ui::Sprite*>    spriteMap;
    ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layouts/menu_view.xml"), spriteMap);
    // Note that the home button is given the name of the xml "home" plus it's local name.
    // This allows you to have multiple instances of the same loaded xml and address all of them
    mHomeButton = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["home.the_button"]);
    if(mHomeButton){
        mHomeButton->setClickFn([this]{ /* do something to load the home screen */ });
    }
    mMapButton = dynamic_cast<ds::ui::SpriteButton*>(spriteMap["map.the_button"]);
    if(mMapButton){
        mMapButton->setClickFn([this]{ /* do something to load the map screen */ });
    }
```

Animation
==============================
You can supply an animation script for the sprite to run when tweenAnimateOn() is called. This allows you to create animations for the interface right from the XML, and tweak them while the app is running. There are some limitations to this method, but should work for most circumstances. You'll supply a simple script that calls the basic tween functions from SpriteAnimatable.

Syntax
------------------------------------------
Animation scripts follow this syntax for each tween (valueY and valueZ are optional):

    <type>:<valueX, valueY, valueZ>;

So for example, this would tween the sprite to ci::Vec3f(100.0f, 200.0f, 300.0f):

    "position:100, 200, 300;"

Tween types can be chained together:

    "position:100, 200, 300; opacity:1.0; scale:1.0, 1.0, 1.0"

    "position:0.0, 100; opacity:0.5; scale:1.0, 1.0; ease:inOutBack; duration:0.5; delay:1.1"

**Basic tween types:**
* **scale**
* **position**
* **opacity** (only the x-value is used for the opacity, y and z are ignored)
* **color** (x,y,z map to r,g,b, respectively, and use 0.0-1.0 values (not 0-255))
* **size** (does not work for some sprite types, like Image)
* **rotation**

**Advanced tween types:**
These tweens move to the current value offset by the supplied value. The first time these are called, the destination positions are cached, so these can be called multiple times. If the destination should be moved, set the sprite to the destination and call setAnimateOnTargets().
* **fade:** Tweens to the current value, and starts at an offset supplied. For instance, fade:-1.0 would tween the opacity of a sprite from 0.0 to 1.0 (assuming it started at 1.0). fade:-0.5 would tween the sprite's opacity from 0.5 to 1.0.
* **slide:** Tweens to the current position, and starts offset by the amount supplied. "slide:-100, 0, 0" would offset the sprite 100 pixels to the left, then tween to the current position.
* **grow:** Tweens to the current scale, offset by the amount supplied.

**Easing:**
Supply a string for the easing type desired. Default is inOutCubic.

    easing:inOutBack;

Valid types:
* **none** = ci::easeNone;
* **inQuad** = ci::easeInQuad;
* **outQuad** = ci::easeOutQuad;
* **inOutQuad** = ci::easeInOutQuad;
* **inCubic** = ci::easeInCubic;
* **outCubic** = ci::easeOutCubic;
* **inOutCubic** = ci::easeInOutCubic;
* **inQuart** = ci::easeInQuart;
* **outQuart** = ci::easeOutQuart;
* **inOutQuart** = ci::easeInOutQuart;
* **inQuint** = ci::easeInQuint;
* **outQuint** = ci::easeOutQuint;
* **inOutQuint** = ci::easeInOutQuint;
* **inSine** = ci::easeInSine;
* **outSine** = ci::easeOutSine;
* **inOutSine** = ci::easeInOutSine;
* **inExpo** = ci::easeInExpo;
* **outExpo** = ci::easeOutExpo;
* **inOutExpo** = ci::easeInOutExpo;
* **inCirc** = ci::easeInCirc;
* **outCirc** = ci::easeOutCirc;
* **inOutCirc** = ci::easeInOutCirc;
* **inBounce** = ci::EaseInBounce();
* **outBounce** = ci::EaseOutBounce();
* **inOutBounce** = ci::EaseInOutBounce();
* **inBack** = ci::EaseInBack();
* **outBack** = ci::EaseOutBack();
* **inOutBack** = ci::EaseInOutBack();
* **inAtan** = ci::EaseInAtan();
* **outAtan** = ci::EaseOutAtan();
* **inOutAtan** = ci::EaseInOutAtan();

**Duration:**
Supply a value for the duration of this tween in seconds. Default is 0.35 seconds.

    duration:1.0;

**Delay:**
Supply a delay in seconds for the start of the tween. Default is 0.0 seconds.

    delay:0.4;

**Cascading delays:**
When calling tweenAnimateOn(), you can optionally supply a delay and a delta delay. The delta delay is added to the delay for each child sprite. This enables a more staggered animation.


ContentModel : model in a SmartLayout
=========================

If you're using ds::model::ContentModelRef for your data model and queries (see Content Model doc) and SmartLayouts for your layouts, you can apply the content models in the XML itself. You'll do this with the **model** property. **Note:** the sprite **must** be named for this value to be read.

* **model**: String, colon-separated sprite parameters, semi-colon and space separated for multiple settings.

**Syntax**: {sprite property}:{content model reference}->{content model property}.

**Example**: 

```XML
<text name="must_name_your_sprite"
	font="sample:font"
	model="text:this->title"
	/>
```

When setting a ContentModelRef, you first specify the **sprite property**. Nearly any sprite property in the above works. Setting the model uses the same code path as the initial parsing. You can set the position, color, font, animation, tap events, text, image source, etc. The advantage of this solution is the ability to put more ui control in the database, allowing for easier re-skinning and tweaking. 

The second part of the syntax is the **content model reference**. Typically you'll use the "this" value, which indicates the current ContentModelRef for this SmartLayout. You can also access any level of children that can be accessed through mContentModel.getChildByName(""), such as "slide.theme" or "sqlite.settings".

After a pointer arrow, you'll specify the **content model property** to use. In general these will be the column names from a sqlite db. Since all data in ContentModelRef is stored as string and type converted when applied, you can apply any content model property to any sprite property, so it's up to you to make sure it makes sense. On the flip side, you could apply properties to a text field for quick debugging. For instance, if a color is not appearing correctly, you could apply the color property to a text field to check the value.

Set multiple models separated by a **semi-colon and a space**.

```cpp
ds::model::ContentModelRef sampleSlideModel = ds::model::ContentModelRef("sample");
sampleSlideModel.setProperty("title", std::string("Hello there!"));

ds::model::ContentModelRef themeModel = ds::model::ContentModelRef("theme");
themeModel.setProperty("title_color", std::string("red"));

sampleSlideModel.addChild(themeModel);

auto mySlide = new ds::ui::SmartLayout(mEngine, "slide.xml");
mySlide->setContentModel(sampleSlideModel);
addChildPtr(mySlide);
```

```XML
<!-- slide.xml -->
<layout name="root_layout" >
	<text name="the_title"
		font="slide:title"
		model="color:theme->title_color; text:this->title"
		/>
</layout>
```

Caveats
-----------------------
Setting the model runs when you call setContentModel() on the SmartLayout. This means that it will be applied after all the other properties on the sprite. Expressions and parameters are evaluated both before and after the model is processed, so this could be valid: model="text:this->$_title_column_name", assuming "title_column_name" is in app_settings.xml. Additionally, you can put expressions and parameters into the content model itself, and they will be evaluated normally. The same limitations apply (e.g. no nested expressions).

**name**: Must have a value for the sprite to have a model applied
**resource and resource_cache**: These currently only apply to Images
**media_player_src**: Tried to find an existing resource or looks it up on disk, but can apply to all media player types
**text_update**: Very handy if you only want to overwrite the text if it exists. This is great if you have a text field with generic info that could show an error message if it's available. Or you could have one layout file for several different tables and only apply the title field text if it exists:

```XML
<text name="the_title"
	font="slide:title"
	model="text_update:this->title; text_update:this->name; text_update:this->label"
	/>
```


