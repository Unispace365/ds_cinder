# From https://github.com/FabriceSalvaire/mupdf-cmake/blob/cmake/cmake/modules/FindOpenJPEG2.cmake
####################################################################################################
#
# - Find JBIG2DEC
# Find the native JBIG2DEC includes and library
# This module defines
#  JBIG2DEC_INCLUDE_DIR, where to find jbig2.h.
#  JBIG2DEC_LIBRARIES, the libraries needed to use JBIG2DEC.
#  JBIG2DEC_FOUND, If false, do not try to use JBIG2DEC.
# also defined, but not for general use are
#  JBIG2DEC_LIBRARY, where to find the JBIG2DEC library.
#
# Written by Fabrice Salvaire
#
####################################################################################################

find_path(JBIG2DEC_INCLUDE_DIR jbig.h)

set(JBIG2DEC_NAMES ${JBIG2DEC_NAMES} jbig2dec)
find_library(JBIG2DEC_LIBRARY NAMES ${JBIG2DEC_NAMES})

# handle the QUIETLY and REQUIRED arguments and set JBIG2DEC_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JBIG2DEC DEFAULT_MSG JBIG2DEC_LIBRARY JBIG2DEC_INCLUDE_DIR)

if(JBIG2DEC_FOUND)
  set(JBIG2DEC_LIBRARIES ${JBIG2DEC_LIBRARY})
endif()

mark_as_advanced(JBIG2DEC_LIBRARY JBIG2DEC_INCLUDE_DIR)

####################################################################################################
#
# End
#
####################################################################################################
