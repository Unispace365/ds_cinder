# From https://github.com/tkrotoff/QuarkPlayer/blob/master/cmake/modules/FindMediaInfoLib.cmake

# - Try to find MediaInfoLib
# Once done this will define
#
#  MEDIAINFOLIB_FOUND - System has MediaInfoLib
#  MEDIAINFOLIB_INCLUDE_DIRS - MediaInfoLib include directories
#  MEDIAINFOLIB_LIBRARIES - Link these to use MediaInfoLib
#  MEDIAINFOLIB_DEFINITIONS - Compiler switches required for using MediaInfoLib

# Copyright (C) 2010  Tanguy Krotoff <tkrotoff@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


find_path(MEDIAINFOLIB_INCLUDE_DIRS
  NAMES
    MediaInfo/MediaInfo.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
)

find_library(MEDIAINFOLIB_LIBRARIES
  NAMES
    mediainfo
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MediaInfoLib DEFAULT_MSG MEDIAINFOLIB_INCLUDE_DIRS MEDIAINFOLIB_LIBRARIES)
