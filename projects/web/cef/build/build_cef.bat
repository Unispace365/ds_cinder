set GN_DEFINES=proprietary_codecs=true ffmpeg_branding=Chrome is_official_build=true
set GN_ARGUMENTS=--ide=vs2017 --sln=cef --filters=//cef/*
python c:\code\automate-git.py --download-dir=C:\code\cg --depot-tools-dir=c:\code\dt --x64-build --branch=3945 --force-clean
pause