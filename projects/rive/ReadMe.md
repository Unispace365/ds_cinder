### How to build the Rive runtime on Windows

 * Make sure to have Visual Studio 22 installed with [Clang++](https://learn.microsoft.com/en-us/cpp/build/clang-support-msbuild) support
 * Clone the repository at https://github.com/rive-app/rive-runtime.git
 * Browse to the `./build` directory
 * Open [Git Bash](https://git-scm.com/downloads) here
 * Type `./build_rive.bat` and press Enter
 * The script now downloads, compiles and installs the dependencies
 * If you run into the following error: "Error: invalid option 'out'":
  * Make sure you have [Premake](https://premake.github.io/download/) installed
  * just run the premake command from the Git Bash command line: './premake5.exe vs2022 --config=release --out=out/release --with_rive_text --with_rive_layout`
 * You should now have a Visual Studio solution that you can open and build