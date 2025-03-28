## What is Rive?

[Rive](https://rive.app/) allows designers and animators to create vector graphics, animate them and even make the animations react to mouse, touch or other inputs. Powerful animation tools and support for state machines give designers all the tools to build what they envision, without the involvement of a developer to translate all that work to code. A Rive file is small and loads very quickly, and thanks to the use of vector graphics can run on any resolution. Click [here](https://rive.app/renderer) to learn more.

## Adding support for Rive to your project

Please follow [these instructions](https://github.com/Unispace365/ds_cinder/blob/develop/projects/README-projects.md) if you want to add Rive to your project.

> [!IMPORTANT]
> Currently, Rive depends on the NvPath project, so make sure to add NvPath to your solution as well.

> [!TIP]
> You may have to force the linker to handle multiple defined symbols, due to Rive using its own copy of the Yoga layout library that is also in DS Cinder. To do this, right-click your project in Visual Studio's solution explorer, then select "Properties". Under `"Configuration Properties" -> "Linker" -> "General"`, find the "Force File Output" option and set it to "Multiply Defined Symbol Only (/FORCE:MULTIPLE)". Do this for all Configurations (Debug, Release, etc.).

## Compiling Rive

Pre-compiled run-times have been added to this repository, but if you want to update Rive, here's how to do it.

### How to build the Rive runtime on Windows

- [ ] Make sure to have Visual Studio 22 installed with [Clang++](https://learn.microsoft.com/en-us/cpp/build/clang-support-msbuild) support
- [ ] Clone the Rive Runtime [repository](https://github.com/rive-app/rive-runtime.git)
- [ ] Browse to the `./build` directory
- [ ] Open [Git Bash](https://git-scm.com/downloads) here
- [ ] Type `./build_rive.bat` and press Enter
- [ ] The script now downloads, compiles and installs all the dependencies
- [ ] Assuming premake5 was built correctly in the previous step, it can be found here: `./dependencies/premake-core/bin/release/premake5.exe`.
- [ ] If you run into the following error: "Error: invalid option 'out'" and/or "Error: invalid option 'config'":
   - [ ] Run the following call from the Git Bash command line: `./dependencies/premake-core/bin/release/premake5.exe vs2022 --with_rive_text --with_rive_layout`
- [ ] A `rive.sln` and `rive.vcxproj` file can now be found in your ./build folder.
- [ ] Open the solution in Visual Studio.
- [ ] Make sure to set "Runtime Library" to "Multi-threaded Debug (/MTd)" for the Debug configuration. You can find it under `"Configuration Properties" -> "C/C++" -> "Code Generation"`.
- [ ] Build all configurations.
- [ ] Copy the files to this repository.

## Rive Sprites

Try playing Rive sprites by drag and dropping `.riv` files onto your application. Just add this code to your main Application class:
```cpp
void YourApp::fileDrop(ci::app::FileDropEvent event) {
  // Allow users to drag-and-drop assets onto the window.
  for (auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it) {
    // Rive support.
    if (it->extension() == ".riv") {
      // mRiveSprite is of type ds::ui::Sprite*
      if (mRiveSprite) mRiveSprite->release();
      mRiveSprite = mEngine.getRootSprite().addChildPtr(new ds::ui::RiveSprite(mEngine, it->string().c_str()));
      break; // Only the first one.
    }
  }
}
```

> [!NOTE]
> Support for Rive is currently under development. Please be aware that:
> * Not all files are playing correctly.
> * Blend Modes (e.g. lighten, overlay) are currently not supported.
> * [Vector Feathering](https://rive.app/blog/introducing-vector-feathering) is not supported.
> * [Inputs](https://rive.app/docs/editor/state-machine/inputs) and [Events](https://rive.app/docs/editor/events/overview#events-overview) are currently not supported.
> * There's currently only a single renderer implementation, using NvPath to render vector graphics. Support for Rive's built-in render-to-image will be added later.
