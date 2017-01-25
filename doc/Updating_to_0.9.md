### Updating to 0.9.0

* Create DS_PLATFORM_090 env variable pointing to the root of this repo
* Create CINDER_090 env variable point to the root cinder_0.9.0_vc2013 folder

* Copy utility/update_to_0.9.ps1 to the root directory of your project and run it with powershell. This script updates the following:
	* Vec3f, Vec3i, Vec2f, Vec2i
	* Macro that launches the app
	* Cinder app basic #include
	* NOTE: You'll likely need to add this to your main app file's cpp: #include <cinder/app/RendererGl.h>

* Global Changes
	* References to cinder::AppBase need to change to cinder::App
	* Cinder readmes: 
	    * Overall 0.9 readme: https://libcinder.org/notes/v0.9.0
		* Transitioning: https://libcinder.org/docs/guides/transition_0_9/index.html
		* OpenGL overview: https://forum.libcinder.org/topic/opengl-in-cinder-0-9-0
		* OpenGL tutorial: https://libcinder.org/docs/guides/opengl/index.html
		
	* Convert Vec2f to vec2, Vec3f to vec3, Vec2i to ivec2, Vec3i to ivec3, Matrix4x4 to mat4, Matrix3x3 to mat3
	* Many math functions have moved to glm, such as distance, lerp, length, etc. Common replacements:
		* mVector3Thing.xy() is now ci::vec2(mVector3Thing);
		* ti.mCurrentGlobalPoint.distance(ti.mStartPoint) is now glm::distance(ti.mCurrentGlobalPoint, ti.mStartPoint)
		* use of lerp might require the inclusion of #include <glm/gtx/compatibility.hpp>
	* OpenGL updates: 
		* If you don't have any custom shaders, you can just delete the entire data/shaders directory
		* All immediate mode OpenGL will need to be rewritten (see OpenGL guides above)

* Text: Pango
	* If you haven't updated your app to handle Pango text, you'll need to do that
	* Fonts are referred to by name, not by a specific alias we used to give them
		* Double-click each font file and install it in Windows Explorer
		* Run your app, you'll likely still see errors. Hit the 'p' key. This will print out the name of every font
		* **Likely to change soon** When you "install" a font on the engine, you'll need to refer to the name of the font, not the path. For example, use "Noto Sans Bold" instead of something like "%APP%/data/fonts/notosans-bold.otf"
		* You can load specific fonts at runtime using mEngine.getPangoFontService().loadFont(ds::Environment::expand("%APP%/data/fonts/notosans-bold.otf"))); After which you'll still need to refer to the fonts by name.
	* All Text sprites are the same now. Text, MultilineText and TextPango all extend the same TextPango sprite now
	* setResizeToText() and autoResize*() functions have been removed. All text sprites now resize to the size of the text they contain
	* The biggest change is that align center now properly centers text in the middle of setResizeLimit()'s width property. This means if you haven't set the resize limit of center-aligned text sprite, you'll need to do that
	* Justify alignment is now available
	* A lightweight markup language is available (see ds/ui/sprite/text_pango.h for usage)
	* Leading and font size should match pretty closely to the previous implementation, though there might be slight rendering differences
	* When a ds_cinder app runs for the first time on a machine, or after a new font has been added, pango will need to scan the fonts on the system to create a font map. This could take a long time, up to a minute or two on slow systems with lots of fonts. After the initial build, subsequent font map builds will be fast.
	
* Potential trouble spots
	* SQlite has been updated, so if you're using any uncommon sqlite functions (FTS for example), you'll want to check those
	* If you were using an uncommon startup macro, you may need to update it to something like this: CINDER_APP( ExampleApp,  ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)) )
	* Borderless windows and Filedrop: If your app has drag-n-drop file abilities and uses a borderless window, you'll need to have the window be set to borderless in the startup macro, like this:
		CINDER_APP(YOURAPP_CLASS_NAME, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)), 
		   [&](ci::app::App::Settings* settings){ settings->setBorderless(true); })