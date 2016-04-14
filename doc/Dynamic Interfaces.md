Basic XML interface
======================

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

Creating a Sprite and loading XML
-----------------------
The tag value is the sprite type. For instance, <sprite/> creates a blank ds::ui::Sprite. Sprites are created as parents/children based on the XML hierarchy. Positions work relative to the parent (like normal).

Loading an interface XML to your class:

    std::map<std::string, ds::ui::Sprite*>	spriteMap;
    ds::ui::XmlImporter::loadXMLto(this, ds::Environment::expand("%APP%/data/layout/layout_view.xml"), spriteMap);
	
Adds the hierarchy in the interface as a child of "this".
Use the spriteMap to look up specific sprites by string name. This lets you apply a data model or deal with touch callbacks, etc. Avoid setting the layout in c++ land, as that can easily confuse where the layout is managed.

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
* **text** = ds::ui::Text
* **multiline_text** = ds::ui::MultilineText
* **gradient** = ds::ui::GradientSprite
* **layout** = ds::ui::LayoutSprite
* **circle** = ds::ui::Circle
* **border** = ds::ui::Border
* **scroll_list** = ds::ui::ScrollList
* **scroll_area** = ds::ui::ScrollArea
* **scroll_bar** = ds::ui::ScrollBar
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
* **transparent**: A boolean of wheather the sprite should draw or not.
* **animate_on**: Supply a script to run when tweenAnimateOn() is called on this sprite. See the animation section for details.

Layout Parameters (only valid if using a layout sprite as a parent)
------------------------------------------------------
* **t_pad**: Padding on the top part of this sprite in the layout
* **b_pad**: Padding on the bottom part of this sprite in the layout
* **l_pad**: Padding on the left part of this sprite in the layout
* **r_pad**: Padding on the right part of this sprite in the layout
* **layout_fudge**: An offset in pixels for the sprite during the layout. Doesn't get added to the overall layout positioning, but allows you to fudge the position a bit if something isn't rendering just right
* **layout_size**: Desired size for the "fixed" layout size mode. This is separate from the sprite size, since some sprites (like Image) will be able to calculate size only after the thing is loaded, and this will try to scale the image (or other thing) up to fit in this size, letterboxed.
* **layout_size_mode**: The method to calculate the size of this sprite during layout
    1. fixed: (Default) The size of this sprite is not modified during the layout
    2. flex: Sprite will be made wide enough (for V layouts) or tall enough (for H layouts) during layout. Images will be sized proportionally to fit the size. MultilineText fields in Vertical Layouts will be resized to the width of the layout (minus padding), and the height calculated (plus padding). MultilineText fields in Horizontal layouts will be resized to their existing resize width limit and constrained to the height of the layout (including padding).
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

Text Parameters (valid for Text and MultilineText Sprites)
------------------------------------------------------------
* **text**: Set the content show on the screen. text="Hello World"
* **font**: The text config. Set in settings/text.xml. The text config sets the font name, size, leading and color. font="sample:config"
* **font_name**: The name of the font registered in the app. **Note:** It's recommended you use the font setting above (a whole config) OR font_name and font_size, and not mix the two.
* **font_size**: Replace the original font size of Text/MultilineText sprites. font_size="20"  
* **font_leading**: For MultilineText, the multiplier of font_size to use for line height
* **resize_limit**: Only has effect on MultitlineText. If you set only the width value, the layout will continue for all the text in the sprite, and the height will be calculated from that. resize_limit="400" or resize_limit="400, 500"
* **text_align**: Set the alignment of a multiline text sprite. Values:
    1. "left": The default, normal text
    2. "center": Center-aligns the layout.
    3. "right": Align rows of text to the right side of the resize_limit

Image Parameters
-------------------------
* **filename** OR **src**: File path RELATIVE to XML. For instance: src="../data/images/refresh_btn.png"
* **circle_crop**: Boolean. If true, will crop image content outside of an ellipse centered within the bounding box.

Image Button Parameters
-------------------------
* **filename**: Will apply the same image to down and up state. Will override down_image and up_image, and vice-versa.
* **down_image**: The image to display when touch is happening on this button. See filename for explanation. down_image="../data/images/refresh_btn.png"
* **up_image**: The image to display when touch is happening on this button. See filename for explanation. up_image="../data/images/refresh_btn.png"
* **btn_touch_padding**: How much space to add to the sprite on every side of the image. btn_touch_padding="40"
* **down_image_color**: Applies a color to the down image, to make creating responsive buttons easy from a single image.


Sprite Button Parameters
----------------------------
**attach_state**: Add this to a sprite that's a child of a sprite button. Valid values: "normal" for the unpressed state and "high" for the pressed state. For example:

    <interface>
        <sprite_button
            name="sample_button" 
            size="400, 200" >
            <text  name="child_text" font="sample:config" text="The Button" />
            <image name="child_image" filename="%APP%/data/images/icons/up_icon.png" attach_state="normal" />
            <image name="child_image" filename="%APP%/data/images/icons/down_icon.png" attach_state="high" />
        </sprite>
    </interface>
	

Gradient Sprite Parameters
---------------------------
* **colorTop**: A color value to set the TL and TR colors of the gradient to. colorTop="ffffff" 
* **colorBot**: A color value to set the BL and BR colors of the gradient to. colorBot="ffffff" 
* **colorLeft**: A color value to set the TL and BL colors of the gradient to. colorLeft="ffffff" 
* **colorRight**: A color value to set the TR and BR colors of the gradient to. colorRight="ffffff" 
* **gradientColors**: Set all four colors for the gradient, specified clockwise from TL "[colorTL], [colorTR], [colorBR], [colorBL]". Example: gradientColors="#ff0000, #000000, #00ff00, #0000ff"

Circle Sprite Parameters
---------------------------
* **filled**: Boolean, whether to draw just the outline or fill in the circle
* **radius**: Float, the radius of the circle to draw

Border Parameters
---------------------------
* **border_width**: Float, the width of the border, which has its outer edge at the extent of the sprite

Scroll List Parameters
-------------------------------
* **Note:** You'll need to supply the usual callbacks for this to work (for creating items in the list, setting data, etc)
* **scroll_list_layout**: Sets the parameters for layout from the format "x, y, z", which translates to setLayoutParams(xStart, yStart, incrementAmount, true);
* **scroll_list_animate**: Sets the animation parameters, from the format "x, y", where x==startDelay and y==deltaDelay on ScrollList::setAnimateOnParams(startDelay, deltaDelay);
* **scroll_fade_colors**: **Also applicable to ScrollArea**. Set the colors of the scroll area, in the format "[colorFull], [colorTransparent]". Example: scroll_fade_colors="ff000000, 00000000" or scroll_fade_colors="44000000, 000000"

XML
-------------------------------
You can load another xml interface from within an xml interface. This is super handy for menus and such that have a bunch of identical buttons or to make a consistent close button for your whole app. 
Limitations:
* You cannot load the same interface recursively, it'd be infinite
* You can't add children to the loaded interface from the parent interface, since the child interface might have many children

Properties:
* You can set the properties of any loaded child from the parent with a "property" tag.
* Children are given a dot naming scheme, and calling properties uses names relative to the child's interface.

Example:
menu_view.xml:

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
	
menu_button.xml:

    <sprite_button name="the_button" size="80, 80">	
        <gradient name="down_gradient" attach_state="high" size="80, 80" 
                  gradientColors="red_orange, red_orange, red_orange, orange"/>
        <image attach_state="normal" name="normal_icon"/>
        <image attach_state="high" name="high_icon" />
    </sprite_button>
	
In c++:

    std::map<std::string, ds::ui::Sprite*>	spriteMap;
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
