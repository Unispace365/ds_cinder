@echo off
@setlocal

rem prevent an error only present on HP computers
set Platform=
set platformcode=

:permissions
rem check admin permissions
net session >nul 2>&1
if not errorlevel 1 goto environment
echo Run this script as administrator.
goto done 

:environment
echo.
echo Preparing environment...
setlocal enableextensions
pushd "%~dp0"

set DS_PLATFORM_093=%~dp0

if not exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
echo Please install Visual Studio 2022 Community prior to running this script.
goto done
)

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 goto error
echo Done.
goto lfs

:lfs
echo.
echo Preparing Git LFS...
cmd /c git lfs install
if errorlevel 1 goto error
cmd /c git lfs fetch --all
if errorlevel 1 goto error
echo Done.
goto submodules

:submodules
echo.
echo Updating Git submodules...
cmd /c git submodule update --init --recursive
if errorlevel 1 goto error
echo Done.
goto clean

:clean
echo.
cmd /c git diff-index --quiet HEAD --
if not errorlevel 1 goto cinder
echo Your working directory is not clean. Please commit all changes first.
goto done

:cinder
echo.
echo Compiling Cinder source...
cmd /c msbuild ".\Cinder\proj\vc2019\cinder.sln" /m /p:Configuration=Debug
if errorlevel 1 goto error
cmd /c msbuild ".\Cinder\proj\vc2019\cinder.sln" /m /p:Configuration=Release
if errorlevel 1 goto error
echo Done.
goto ds

:ds
echo.
echo Compiling DS Cinder...
cmd /c msbuild ".\vs2015\platform.vcxproj" /m /p:Configuration=Debug
if errorlevel 1 goto error
cmd /c msbuild ".\projects\web\cef\cef_web.vcxproj" /m /p:Configuration=Debug
if errorlevel 1 goto error
cmd /c msbuild ".\projects\essentials\essentials.vcxproj" /m /p:Configuration=Debug
if errorlevel 1 goto error
cmd /c msbuild ".\projects\pdf\mupdf\pdf.vcxproj" /m /p:Configuration=Debug
if errorlevel 1 goto error
cmd /c msbuild ".\projects\physics\box2d\physics.vcxproj" /m /p:Configuration=Debug
if errorlevel 1 goto error
cmd /c msbuild ".\projects\video\gstreamer-1.0\video.vcxproj" /m /p:Configuration=Debug
if errorlevel 1 goto error
cmd /c msbuild ".\projects\viewers\viewers.vcxproj" /m /p:Configuration=Debug
if errorlevel 1 goto error
cmd /c msbuild ".\vs2015\platform.vcxproj" /m /p:Configuration=Release
if errorlevel 1 goto error
cmd /c msbuild ".\projects\web\cef\cef_web.vcxproj" /m /p:Configuration=Release
if errorlevel 1 goto error
cmd /c msbuild ".\projects\essentials\essentials.vcxproj" /m /p:Configuration=Release
if errorlevel 1 goto error
cmd /c msbuild ".\projects\pdf\mupdf\pdf.vcxproj" /m /p:Configuration=Release
if errorlevel 1 goto error
cmd /c msbuild ".\projects\physics\box2d\physics.vcxproj" /m /p:Configuration=Release
if errorlevel 1 goto error
cmd /c msbuild ".\projects\video\gstreamer-1.0\video.vcxproj" /m /p:Configuration=Release
if errorlevel 1 goto error
cmd /c msbuild ".\projects\viewers\viewers.vcxproj" /m /p:Configuration=Release
if errorlevel 1 goto error
echo Done.
goto done

:error
echo An error occurred. See README.md for more information.

:done
echo.
pause