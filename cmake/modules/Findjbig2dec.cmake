# - Try to find jbig2dec
# Once done, this will define:
#
#  JBIG2DEC_FOUND - system has jbig2dec
#  JBIG2DEC_INCLUDE_DIRS - the jbig2dec include directories
#  JBIG2DEC_LIBRARIES - link these to use jbig2dec

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(JBIG2DEC_PKGCONF libjbig2dec)

# Include dir
find_path(JBIG2DEC_INCLUDE_DIR
  NAMES jbig2.h
  HINTS ${JBIG2DEC_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(JBIG2DEC_LIBRARY
  NAMES jbig2dec
  PATHS ${JBIG2DEC_PKGCONF_LIBRARY_DIRS}
)
