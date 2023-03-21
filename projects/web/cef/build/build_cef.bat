set GN_DEFINES=is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome
set GYP_MSVS_VERSION=2019
set CEF_ARCHIVE_FORMAT=tar.bz2
python automate-git.py --download-dir=c:\code\cg --depot-tools-dir=c:\code\dt --branch=5481 --minimal-distrib --client-distrib --force-clean --x64-build --with-pgo-profiles

pause

