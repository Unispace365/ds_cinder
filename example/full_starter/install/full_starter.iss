; ----- Required app parameters

; What displays in the start menu, also used as the Unique app name / id
#define APP_DISPLAY_NAME "FullStarter"
; Bump the version for each output build
#define APP_VERS "1.0.0"
; Used for paths for the app directory. Recommend using the auto-generated values
#define APP_NAME "FullStarter"
; The exe file itself
#define APP_EXE "full_starter.exe"

; ------ Optional Flags

; Setup for the machine in production
;  - Adds env variable for Pango / Cario backend
;  - Starts the app on system boot
;  - Disables Windows Error Reporting UI
#define IS_PRODUCTION

; If your app doesn't use gstreamer videos, you can comment this out by adding a semicolon to the beginning of the line
; Alternatively, you can install GStreamer manually on the destination machine, though this is much more convenient
#define USE_GSTREAMER

; App runs inside apphost
; Automatically add the apphost.json config file to the correct destination directory
; If IS_PRODUCTION is defined, will start apphost on system boot
#define USE_APPHOST

; Uses DsNode to connect to a DsCMS
; If USE_APPHOST is also defined, will run dsnode inside Apphost
#define USE_DSNODE

; Sets the DS_BASEURL env variable for dsnode if present AND IS_PRODUCTION is defined
;#define CMS_URL ""        

; Define to not create a normal app icon
;#define SKIP_APP_ICON

; -------- Required include of the base install
; This grabs the full installer script to actually build the thing
; Check that file if you have any problems or questions
; App binaries, data, and settings (minus configuration.xml) are automatically added
#include GetEnv('DS_PLATFORM_090') + "/install/base_install.iss"


; -------- Optional additional stuff
; Add any additional files, settings, icons, registry, etc here
;[Files]
;Source: "src/*"; DestDir: "{app}/src"; Flags: recursesubdirs