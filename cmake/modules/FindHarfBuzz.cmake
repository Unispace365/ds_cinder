# Try to find Harfbuzz include and library directories.
#
# After successful discovery, this will set for inclusion where needed:
# HARFBUZZ_INCLUDE_DIRS - containg the HarfBuzz headers
# HARFBUZZ_LIBRARIES - containg the HarfBuzz library

INCLUDE(FindPkgConfig)

PKG_CHECK_MODULES(PC_HARFBUZZ harfbuzz>=0.9.0)

FIND_PATH(HARFBUZZ_INCLUDE_DIRS NAMES hb.h
  HINTS ${PC_HARFBUZZ_INCLUDE_DIRS} ${PC_HARFBUZZ_INCLUDEDIR}
)

FIND_LIBRARY(HARFBUZZ_LIBRARIES NAMES harfbuzz
  HINTS ${PC_HARFBUZZ_LIBRARY_DIRS} ${PC_HARFBUZZ_LIBDIR}
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HarfBuzz DEFAULT_MSG HARFBUZZ_INCLUDE_DIRS HARFBUZZ_LIBRARIES)
