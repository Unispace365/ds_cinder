# MuPDF and you

> http://www.mupdf.com/
> Current MuPdf version: 1.21.x

## Building MuPdf - Roughly following [MuPdf Building Guide](https://www.mupdf.com/docs/building.html)
1. Clone MuPdf `git clone --recursive git://git.ghostscript.com/mupdf.git`
2. Check out the current release branch (1.21.x as I'm writing this)
3. Pull the submodules `(git submodule update --init)`
4. Open `mupdf/project/win32/mupdf.sln` in Visual Studio
5. Right click the libmupdf project -> Properties -> General -> Set platform toolset to v143 (Visual Studio 2022)
6. In the same window ensure the windows SDK version is set to latest
7. Build the libmupdf *Solution* in both Debug(x64) and Release(x64)
8. Assuming all goes well, you're ready to plug it in to DsCinder!

## Updating DsCinder mupdf libs
1. Copy the contents of `mupdf/include` to `ds_cinder/projects/pdf/mupdf/lib/MuPdf` (should overwrite the existing files)
2. From `mupdf/project/win32/x64/Debug/` copy libmupdf.lib and libthirdparty.lib to `ds_cinder/projects/pdf/lib/MuPDF/lib64/debug/v143/`
3. From `mupdf/project/win32/x64/Release/` copy libmupdf.lib and libthirdparty.lib to `ds_cinder/projects/pdf/lib/MuPDF/lib64/release/v143/`
4. Finally, copy `mupdf/project/win32/Release/libresources.lib` to `ds_cinder/projects/pdf/lib64/`

## Building and Testing an app
1. Do a clean + build in both Debug and Release
2. Make sure the app compiles
	- If you get an error, ensure you built with the correct platform toolset and SDK version
	- Make sure all the files were copied successfully
3. Run the app in both Debug and Release and ensure that PDFs load and render as expected

## Finishing Touches
1. Update this readme with the version of MuPDF you used (and any changes required to make things work!)
2. Commit your changes to a branch and create a Pull Request to merge back to the branch you were updating
3. Pour yourself a nice drink, give yourself a pat on the back, and bask in the blinding light of your skills :)

:heart:
