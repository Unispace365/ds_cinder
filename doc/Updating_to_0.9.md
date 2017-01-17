### Updating to 0.9.0

* Copy utility/update_to_0.9.ps1 to the root directory of your project and run it with powershell. This script updates the following:
	* Vec3f, Vec3i, Vec2f, Vec2i
	* Macro that launches the app
	* Cinder app basic #include
	*

* Global Changes
	* Cinder readmes: 
	    * Overall 0.9 readme: https://libcinder.org/notes/v0.9.0
		* Transitioning: https://libcinder.org/docs/guides/transition_0_9/index.html
		* OpenGL overview: https://forum.libcinder.org/topic/opengl-in-cinder-0-9-0
		* OpenGL tutorial: https://libcinder.org/docs/guides/opengl/index.html
		
	* Convert Vec2f to vec2, Vec3f to vec3, Vec2i to ivec2, Vec3i to ivec3, Matrix4x4 to mat4, Matrix3x3 to mat3
	* OpenGL updates: 
		* If you don't have any custom shaders, you can just delete your data/shaders directory )
		* All immediate mode OpenGL will need to be rewritten (see OpenGL guides above)

* Text: Pango
	* If you haven't updated your app to handle Pango text, you'll need to do that
	* (For now) Fonts need to be installed system-wide and referred to by name
		* Double-click each font file and install it in Windows Explorer
		* Run your app, you'll likely still see errors. Hit the 'p' key. This will print out the name of every font
		* Change the path of each font file to the font name you found in the previous step
			* For example, instead of "%APP%/data/fonts/example-bold.otf" you'll enter something like "Example Bold"
	* All Text sprites are the same now. Text, MultilineText and TextPango all extend the same TextPango sprite now
	* setResizeToText() and autoResize*() functions have been removed. All text sprites now resize to the size of the text they contain
	* The biggest change is that align center now properly centers text in the middle of setResizeLimit()'s width property. This means if you haven't set the resize limit of center-aligned text sprite, you'll need to do that
	* Justify alignment is now available
	* A lightweight markup language is available (see ds/ui/sprite/text_pango.h for usage)
	* Leading and font size should match pretty closely to the previous implementation, though there might be slight rendering differences