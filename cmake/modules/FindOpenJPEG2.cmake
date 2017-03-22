# - Try to find OpenJPEG2
# Once done, this will define:
#
#  OPENJPEG2_FOUND - system has OpenJPEG2
#  OPENJPEG2_INCLUDE_DIRS - the OpenJPEG2 include directories
#  OPENJPEG2_LIBRARIES - link these to use OpenJPEG2

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(OPENJPEG2_PKGCONF libopenjp2)

# Include dir
find_path(OPENJPEG2_INCLUDE_DIR
  NAMES openjpeg.h
  HINTS ${OPENJPEG2_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(OPENJPEG2_LIBRARY
  NAMES openjp2
  PATHS ${OPENJPEG2_PKGCONF_LIBRARY_DIRS}
)

