@echo off

pushd "%~dp0"

goto :boost

:: Check if our working directory is clean.
:clean
cmd /c git diff-index --quiet HEAD --
if errorlevel 1 (
    echo Your working directory is not clean. Please commit all changes first.
    goto :error
)

:: Make sure LFS is initialized and all files have been fetched.
:lfs
cmd /c git lfs install
if errorlevel 1 goto :error
cmd /c git lfs fetch --all
if errorlevel 1 goto :error

:: Make sure we have updated all submodules.
:submodules
cmd /c git submodule update --init --recursive
if errorlevel 1 goto :error

:: Install Boost.
:boost
call "%~dp0build_system\setup_boost.bat" 1.74.0
if errorlevel 1 goto :error

:: Install GStreamer.
:gstreamer
call "%~dp0build_system\setup_gstreamer.bat" 1.24.12
if errorlevel 1 goto :error

::
setlocal EnableDelayedExpansion

:: Prevent an error only present on HP computers
set "Platform="
set "platformcode="

:: Assign variables
set "target=%~1"
set "solution_dir=%~2"

if not defined target set "target=Release"
if defined solution_dir (
    set "solution_dir=%~dpnx2"
    if not "!solution_dir:~-1!"=="\" set "solution_dir=!solution_dir!\\"
)

:: Setup build environment.
:environment
if not exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    echo Please install Visual Studio 2022 Community prior to running this script.
    goto :error
)

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 goto :error

:: Build Cinder.
:build
msbuild ".\Cinder\proj\vc2019\cinder.sln" /m /p:Configuration=%target%
if errorlevel 1 goto :error

:: Build projects.
for %%p in (
    ".\vs2015\platform.vcxproj"
    ".\projects\web\cef\cef_web.vcxproj"
    ".\projects\essentials\essentials.vcxproj"
    ".\projects\pdf\mupdf\pdf.vcxproj"
    ".\projects\physics\box2d\physics.vcxproj"
    ".\projects\video\gstreamer-1.0\video.vcxproj"
    ".\projects\viewers\viewers.vcxproj"
    ".\projects\nvpath\nvpath.vcxproj"
    ".\projects\waffles\waffles.vcxproj"
) do (
    if defined solution_dir (
        echo Compiling to %solution_dir%
        msbuild "%%p" /m /p:Configuration=%target% /p:SolutionDir="%solution_dir%"
        if errorlevel 1 goto :error
    ) else (
        echo Compiling
        msbuild "%%p" /m /p:Configuration=%target% 
        if errorlevel 1 goto :error
    )
)

endlocal

:: Point environment variable to this repository.
set "DS_PLATFORM_093=%~dp0"
setx "DS_PLATFORM_093" "%~dp0" >NUL 2>&1

popd
pause
exit /b 0

:error
echo An error occurred. See README.md for more information.

popd
pause
exit /b 1