# Keyboard Shortcuts

## Basics

* **Escape, Q, F4**: Quit App
* **ctrl-k**: Kill DsNode and Restart on Crash, then this
* **c**: Toggle console
* **h**: Print available keys
* **e**: Toggle settings editor
* **s**: Toggle on-screen stats viewer (number of sprites, touch mode, framerate, memory)
* **r**: Soft-restart the app

## Window / Rendering
* **f**: Toggle fullscreen
* **a**: Toggle always-on-top mode
* **F8**: Take a screenshot and save on desktop
* **m**: Toggle mouse (can be overriden by auto mouse)
* **Arrows**: Move the rendering output around (src_rect shifting)

## App
* **i**: Toggle idling
* **t**: Cycle through touch modes
* **v**: Toggle verbose logging
* **alt-v**: Increasing verbose logging level
* **shift-alt-v**: Decrease verbose logging level
* **d**: Highlight enabled sprites (helpful for touch debugging)
* **ctrl-d**: Log the entire sprite hierarchy
* **n**: Requery content
* **l**: Print content hierarchy to console
* **shift-l**: Verbose print content hierarchy to console
* **p**: Log system fonts, helpful for setting font names in text.xml
* **shift-p**: Verbose logging of system fonts


## Adding your own shortcuts

In the main app class:

```cpp
registerKeyPress("Do the thing", [this] { doTheThing(); }, ci::app::KeyEvent::KEY_n);
```
	
That's it! This will print out with the rest of the key commands when pressing 'h'. There's no check for duplication, so if you duplicate a key, both functions will be called. You can also add optional flags for shift, ctrl, and alt.
