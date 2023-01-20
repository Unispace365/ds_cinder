# Poco and you

> [Poco Project](https://pocoproject.org) version: 1.21.x

## Downloading the Source
> [Poco download page](https://pocoproject.org/download.html)
1. Scroll down to "Signed Packages" and download the Basic Edition
2. Extract the zip file to a handy location

## Building Poco
> You'll need a fairly current copy of [cmake](https://cmake.org/), Visual Studio 2022, and a terminal
1. Start your terminal of choice in the poco-x.xx.x folder you extracted above
2. Run `mkdir cmake-build && cd cmake-build`
3. Run `cmake .. -DPOCO_STATIC=ON -DPOCO_MT=ON` (This configures the project for multithreading and building as a static library)
4. Now to build!
	- Build debug: `cmake --build . --config debug`
	- Build debug: `cmake --build . --config release`
5. Use cmake to "install" the files in a temporary directory. This puts both the lib files and the required headers in a
   single location
	- Run `cmake --install . --config Debug --prefix="${USERPROFILE}/Desktop/poco_temp"`
	- Run `cmake --install . --config Release --prefix="${USERPROFILE}/Desktop/poco_temp"`
6. If all went well you should have a `poco_temp` folder on your desktop

## Updating DsCinder + Testing
1. Replace the include & lib folders in `ds_cinder/lib/poco` with the ones from `poco_temp` on your desktop
2. Delete the `.exp` and `.pdb` files in `ds_cinder/lib/poco/lib`
3. Do a clean build of a ds_cinder project in both Debug and Release and make sure everything still compiles and runs
4. Commit your changes to a branch and create a Pull Request to merge back to the branch you were updating
5. Feel good about all you've accomplished or question your life choices, either way, you're done! :confetti_ball:
