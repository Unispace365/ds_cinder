Basic XML interface
======================

<interface>
	<sprite 
		name="sprite_name" 
		size="400, 200" 
		position="100, 200, 300" 
		color="ff0000" 
		enable="true" 
		multitouch="all" >
		<text name="child_text" font="sample:config" resize_limit="300" />
		<image name="child_image" src="RELATIVE PATH TO IMG" />
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
* **Color**: Format: #AARRGGBB, #RRGGBB, AARRGGBB, or RRGGBB. For instance: color="#FF0033bb" or color="333333".
* **Boolean**: These values will map to true: true, TRUE, yes, YES, on, ON. Everything else is false.
* **Blend**: Valid values: normal, multiply, screen, add, subtract, lighten, darken. Default = normal.
* **Float**: Specify the number with the decimal. The trailing "f" is unneccasary. For instance, opacity="0.5" not opacity="0.5f"

Sprite Types
--------------------------
* **sprite** = ds::ui::Sprite
* **image** = ds::ui::Image
* **text** = ds::ui::Text
* **multiline_text** = ds::ui::MultilineText
* **image_button** = ds::ui::ImageButton
* **gradient** = ds::ui::GradientSprite
* **layout** = ds::ui::LayoutSprite
* **circle** = ds::ui::Circle
* **[custom]** = Calls a custom callback function with a string for the type. Requires you instantiate the sprite type yourself.

Sprite Parameters
-----------------------

* **width**: Specify a float for the width of the sprite. width="100"
* **height**: Specify a float for the height of the sprite. height="100"
* **depth**: Specify a float for the depth of the sprite. depth="100"
* **size**: Specify a 2d vector for the width and height (depth not supported). size="400, 100"
* **color**: Sets the color of the sprite and makes it non-transparent (so it renders). Format: #AARRGGBB, #RRGGBB, AARRGGBB, or RRGGBB. For instance: color="#FF0033bb" or color="333333".
* **opacity**: The opacity as a float from 0.0 to 1.0. opacity="0.5"
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
    3. stretch: Sprite will be expanded to fit the remaining space available in the layout. Adding Stretch layouts to Flex layouts may produces unexpected results. Multiple stretch layouts will evenly split the remaining space.
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
    3. <any other value>: None, does no layout of children.
* **layout_spacing**: For LayoutSprite only, a float that sets the spacing between all elements. layout_spacing="10.0"

Text Parameters (valid for Text and MultilineText Sprites)
------------------------------------------------------------
* **font**: The text config. Set in settings/text.xml. The text config sets the font name, size, leading and color. font="sample:config"
* **resize_limit**: Only has effect on MultitlineText. If you set only the width value, the layout will continue for all the text in the sprite, and the height will be calculated from that. resize_limit="400" or resize_limit="400, 500"

Image Parameters
-------------------------
* **filename** OR **src**: File path RELATIVE to XML. For instance: src="../data/images/refresh_btn.png"

Image Button Parameters
-------------------------
* **down_image**: The image to display when touch is happening on this button. See filename for explanation. down_image="../data/images/refresh_btn.png"
* **up_image**: The image to display when touch is happening on this button. See filename for explanation. up_image="../data/images/refresh_btn.png"
* **btn_touch_padding**: How much space to add to the sprite on every side of the image. btn_touch_padding="40"

Gradient Sprite Parameters
---------------------------
* **colorTop**: A color value to set the TL and TR colors of the gradient to. colorTop="ffffff" 
* **colorBot**: A color value to set the BL and BR colors of the gradient to. colorBot="ffffff" 

Circle Sprite Parameters
---------------------------
* **filled**: Boolean, whether to draw just the outline or fill in the circle
* **radius**: Float, the radius of the circle to draw
