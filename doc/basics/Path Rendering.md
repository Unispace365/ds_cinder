# Path Rendering

## What is it?

Unlike 3D graphics, which rely on rasterization of 3D geometry onto a 2D plane, 2D vector graphics require complex stroke, fill, and text rendering with precise anti-aliasing. 2D vector graphics use mathematical primitives like lines, curves, and shapes rather than pixel-based representations, making them resolution-independent. GPU acceleration is harder to leverage compared to 3D, where shaders and pipelines are optimized for polygons. Rendering performance can suffer from complex path operations or clipping. GPUs are optimized for rasterization of polygons, so rendering smooth vector shapes often involves decomposing paths into triangles (via tessellation) or using fragment shaders to evaluate coverage per pixel.

Between 2012 and 2015, NVIDIA released an OpenGL extension that makes rendering 2D vector graphics much easier. The NVIDIA Path Rendering extension simplifies rendering 2D vector graphics by offloading path processing to the GPU. Instead of manually tessellating vector shapes into triangles, this extension allows direct rendering of paths using GPU-accelerated stroke and fill operations. It provides a scalable GPU pipeline for drawing Bézier curves, lines, and complex shapes with precise anti-aliasing. The extension leverages stencil buffer techniques to efficiently compute path coverage, eliminating the need for software-based path rasterization. It also supports GPU-accelerated transformations, clipping, and text rendering, making it ideal for high-performance vector graphics in OpenGL-based applications.

## NvPath

Using the NVIDIA Path Rendering extension requires careful OpenGL state management, as paths are defined, transformed, and rendered using specific commands. This involves setting up path objects, configuring stencil and coverage buffers, and managing transformation matrices within the OpenGL pipeline. To simplify this, we created a wrapper called "NvPath" that abstracts away the low-level state changes, making it easier to issue path rendering commands without manually handling OpenGL states. This wrapper streamlines tasks like path creation, filling, stroking, and clipping, reducing boilerplate code and ensuring correct OpenGL state transitions for rendering vector graphics efficiently.

## Using NvPath in your project

Just like Waffles, CEF or PDF support, path rendering comes as a separate project in the DS Cinder repository. Just add `[ds_cinder]/proejcts/nvpath/nvpath.vcxproj` to your solution. 

Note: _the NVIDIA Path Rendering extension requires an NVIDIA GPU. Other brands are not supported._

Additionally, make sure your main buffer has the stencil buffer enabled. To do this, change your call to `CINDER_APP` in your main application source file:
```
// This line tells Cinder to actually create the application.
CINDER_APP( YourApp, ci::app::RendererGl( ci::app::RendererGl::Options().stencil() ) )
```

To start drawing paths, make sure to call `nvpath::ScopedPathRendering sp;`. This will setup the proper OpenGL state and afterwards make sure the previous state is properly restored.

## Code Examples

### Primitives

![circle](https://github.com/user-attachments/assets/09135d52-a3b4-440c-89d6-b43c0f5e8a10)
```
#include "nvpath/NvPath.h"

{
  // Setup OpenGL state for path rendering.
  nvpath::ScopedPathRendering sp;

  // Create a circle using a helper function.
  nvpath::Path circle = nvpath::circle( 128, 128, 100 );

  // Fill the circle with a red color.
  circle.fill( ci::Color::hex( 0x990000 ) );
}
```
The following primitives can be created using helper functions: _Circle_, _Ellipse_, _Arc_, _Line_, _Polygon_, _Rounded Polygon_, _Rectangle_, _Rounded Rectangle_, _Star_, _Arrow_.

Note that constructing an `nvpath::Path` on-the-fly is usually very fast, so you can create them on-the-fly and throw them away after use. They don't occupy much CPU memory (just an `int` to store the path's ID), since all the path's data is stored on the GPU. However, if you repeatedly need to draw the same path, it's better to use a member variable to store and reuse the path.

#### Fill

If your path forms a closed shape, you can fill it with a color, a gradient or an image. Simply call the path's `fill` method. Under the hood, the shape is then first rendered to the stencil buffer, creating an anti-aliased mask. Subsequently, bounding geometry is drawn to cover the mask with your color, gradient or image. Only pixels within the mask will be affected. Anything outside the mask remains untouched. The covering step also automatically clears the mask in the stencil buffer (unless otherwise specified), so it's immediately ready to draw the next shape.

#### Stroke

Not only can we fill paths with a color, a gradient or an image (see below), but we can also stroke them. The stroke capabilities of paths are very powerful, allowing us to use any width, determine the way sharp corners are shaped (a.k.a. join style), use dashing patterns and even control the shape of the dash and end caps (a.k.a. caps style).

![146687](https://github.com/user-attachments/assets/5d4971eb-09ca-4c0b-a68c-0e9391a1959e)
```
#include "nvpath/NvPath.h"

{
  // Setup OpenGL state for path rendering.
  nvpath::ScopedPathRendering sp;

  // Create a line using a helper function.
  nvpath::Path line = nvpath::line( 32, 32, 224, 32 );

  // Stroke the line with a blue color.
  line.setStrokeWidth(10);
  line.stroke( ci::Color::hex(0x000099 ) );

  // Move the line down a bit and use rounded end caps to stroke it.
  line.translate(0, 64);
  line.setEndCaps( nvpath::CapsStyle::ROUND );
  line.stroke( ci::Color::hex(0x000099 ) );

  // Move the line down again and apply a dash pattern to stroke it.
  line.translate(0, 64);
  line.setDashPattern( { 12.8f, 12.8f } );
  line.stroke( ci::Color::hex(0x000099 ) );
}
```
For more information on stroke commands, follow this [link](https://www.w3schools.com/graphics/svg_stroking.asp).

##### Precise Fitting of a Dash Pattern

Let's say you'd like to render a rounded rectangle with a dashed outline. If you specify the dash pattern as `path.setDashPattern({5, 3})`, the outline will be dashed using line segments 5px long, with gaps of 3px between the line segments. Chances are this pattern will not cover the full length of the outline precisely, causing the pattern to not line up at the begin and end points.

To fix this, use `path.setDashPatternFitted({5, 3})`. The size of the pattern will be slightly tweaked to make sure it can be repeated an integer number of times on the outline.

### Paths

Instead of using the primitive helper functions to create paths, You can define paths using an easy to learn format described [here](https://www.w3schools.com/graphics/svg_path.asp). It allows you to quickly create elaborate shapes from code, or you can copy-paste path descriptions from SVG files, like we did in the following example.

![path](https://github.com/user-attachments/assets/7d9cccdd-ffc7-4bc3-af27-076138b6dec7)
```
#include "nvpath/NvPath.h"

{
  // Setup OpenGL state for path rendering.
  nvpath::ScopedPathRendering sp;

  // Create a path using SVG notation.
  nvpath::Path heart = nvpath::Path("M398.327,30.737c-50.875,0-95.875,31.151-123.452,69.542c-27.577-38.391-72.577-69.542-123.453-69.542c-43.452,0-85.484,19.04-114.168,51.677c-55.221,62.834-43.085,166.366-2.375,233.454c37.014,60.992,93.832,109.188,151.972,149.303c28.091,19.383,57.387,37.902,88.024,53.055c99.891-47.723,210.791-127.301,255.975-231.268c14.211-32.706,20.074-69.187,17.54-104.126C542.588,102.751,492.717,30.737,398.327,30.737z M474.724,262.563c-29.517,67.915-101.28,134.971-198.998,186.739c-16.542-9.498-34.248-20.795-54.119-34.504c-64.682-44.627-107.394-86.15-134.407-130.674c-15.6-25.704-25.049-57.908-25.931-88.342c-0.612-21.188,2.699-51.047,21.958-72.963c16.996-19.333,42.491-30.881,68.202-30.881c31.046,0,58.452,22.754,73.752,44.045c11.5,16.004,30,43.856,49.707,43.856s38.208-27.852,49.707-43.856c15.294-21.291,42.699-44.045,73.752-44.045c26.77,0,47.24,8.36,62.577,25.557c15.013,16.824,24.4,41.598,26.438,69.756C489.203,212.82,484.718,239.564,474.724,262.563z");

  // Fill the path with a red color.
  heart.fill( ci::Color::hex( 0x990000 ) );
}
```

#### PathHelper

If you find using the SVG path definition too cumbersome to work with, you could also construct a `PathHelper` instance to construct a path. Use its methods to add commands and coordinates, then pass it as a parameter to `Path` to create the actual path.

```
#include "nvpath/NvPath.h"

{
  // Setup OpenGL state for path rendering.
  nvpath::ScopedPathRendering sp;

  PathHelper h;
  h.moveTo( 50, 50 );
  h.lineTo( 150, 50 );
  h.verticalLineTo( 150 );
  h.horizontalLineTo( 50 );
  h.close();

  Path box(h);
  box.fill( ci::Color(1, 0, 0) );
}
```

### Paints and Gradients

More information on how to define gradients and use them to fill paths will follow shortly. For now, have a look at the `nvpath::Paint` class to learn how to construct a gradient. It can then be used as a parameter to the `Path::fill()` command. 

### SVG

Instead of defining your paths along with their colors, gradients and stroke settings, you can also use SVG files. This allows for some very complex content. Our NvPath code can handle most SVG files, with a few exceptions (most notably filters and effects that require compositing, like group opacity). 

More information will follow, but for now, take a look at the SvgSprite class.

### Clip Paths

Paths can also be used to mask content, even basic non-path objects. Just create your path, which can have any shape, and then call `nvpath::pushClipPath( clipPath )` to enable clipping. Subsequent paths and SVG files will be properly clipped by the clip ath until a call to `nvpath::popClipPath()` disables the clip mask again. A maximum number of 5 nested clip paths can be applied concurrently. These will be AND-ed together, causing content to only appear if within *all* clip paths.

![clip-path](https://github.com/user-attachments/assets/10102008-a957-4189-8801-a5bb2ffb1430)
```
#include "nvpath/NvPath.h"

{
  // Setup OpenGL state for path rendering.
  nvpath::ScopedPathRendering sp;

  // Define a path to be used as a mask.
  auto star = nvpath::star(256, 256, 200, 100, 5, 0);

  // Now enable it as a clip path.
  nvpath::pushClipPath(star);

  // Now render a circle, but only the fragments that are inside the mask.
  auto circle = nvpath::circle(256, 256, 150);
  circle.fill( ci::Color::hex( 0x009933 ) );

  // To clip non-path content, we need to set the proper stencil buffer state.
  // This can easily be done using the ScopedStencilState helper:
  nvpath::ScopedStencilState sss( false /* non-path */ );
  ci::gl::ScopedGlslProg sg(ci::gl::getStockShader(ci::gl::ShaderDef().color()));
  ci::gl::ScopedColor	   sc( ci::Color::hex( 0x003399 ) );
  ci::gl::drawSolidRect({256, 0, 512, 512});

  // Now pop the clip mask to clear the stencil buffer.
  nvpath::popClipPath();
}
```
