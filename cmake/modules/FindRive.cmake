# Try to find Rive include and library directories.
#
# After successful discovery, this will set for inclusion where needed:
# RIVE_INCLUDE_DIRS - containg the Rive headers
# RIVE_LIBRARIES - containg the Rive library

INCLUDE(FindPkgConfig)

PKG_CHECK_MODULES(PC_RIVE rive>=0.0.0)

FIND_PATH(RIVE_INCLUDE_DIRS NAMES hb.h
  HINTS ${PC_RIVE_INCLUDE_DIRS} ${PC_RIVE_INCLUDEDIR}
)

FIND_LIBRARY(RIVE_LIBRARIES NAMES rive
  HINTS ${PC_RIVE_LIBRARY_DIRS} ${PC_RIVE_LIBDIR}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Rive DEFAULT_MSG RIVE_INCLUDE_DIRS RIVE_LIBRARIES)
