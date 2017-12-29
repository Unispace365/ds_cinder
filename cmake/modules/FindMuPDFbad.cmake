# - Try to find MuPDF
# Once done, this will define:
#
#  MUPDF_FOUND - system has MuPDF
#  MUPDF_INCLUDE_DIRS - the MuPDF include directories
#  MUPDF_LIBRARY - link these to use MuPDF

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(MUPDF_PKGCONF mupdf)

# Include dir
find_path(MUPDF_INCLUDE_DIR
  NAMES mupdf/mupdf.h
  HINTS ${MUPDF_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES freetype2
)

# Finally the library itself
find_library(MUPDF_STATIC_LIBRARY
  NAMES mupdf
  HINTS ${MUPDF_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this lib depends on.
set(MUPDF_PROCESS_INCLUDES MUPDF_INCLUDE_DIR)
set(MUPDF_PROCESS_LIBS MUPDF_LIBRARY)
libfind_process(mupdf)

