# Building An Installer (using innosetup)

## Prerequisites
- (Inno Setup)[http://www.jrsoftware.org/isdl.php]

## Configuring & Building Installer

If you used the project cloner, everything you need is in `{yourproject}/install`. If not, you can copy `{ds_cinder}/examples/full_starter/install` to your app directory, and configure from there.

The main configuration file for the installer is `install/{project}.iss`. That script sets up the variables and settings for the install, and includes `{ds_cinder}/install/base_install.iss`, which handles the details.

#define            | function
---                | ---
`APP_DISPLAY_NAME` | Name used for app shortcuts, and in the application list
`APP_VERS`         | Version number, in `major revision`.`minor revision`.`bugfix` notation
`APP_NAME`         | name for the app director, prefer using the name of the github project
`APP_EXE`          | The main app executable
`IS_PRODUCTION`    | Sets environment variables typical for production
`USE_GSTREAMER`    | Package gstreamer dlls with the installer. Prefer this to installing gstreamer `manually`
`USE_DSNODE`       | Install dsnode. If IS_PRODUCTION is set, but USE_APPHOST is not, dsnode will be set to run on boot.
`USE_APPHOST`      | Install apphost, and run at boot
`CMS_URL`          | If using ds_node, this sets the DS_BASEURL environment variable.
`SKIP_APP_ICON`    | Don't create desktop shortcut for app. Useful if `USE_APPHOST` is set, which will create an apphost desktop icon.
`USE_EXTRAS`       | Install [ninite](https://ninite.com/) package with 7zip, notepad++ & Chrome. Installer will give the user the option of running ninite once the install completes

- If your app uses dsnode, run the `download_dsnode.ps1` powershell script.
- If you plan on using apphost, run the `download_apphost.ps1` script too.
- Right click your `.iss` file and click compile. The installer is created in `{yourproject}/install/build`
