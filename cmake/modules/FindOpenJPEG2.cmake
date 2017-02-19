# From https://github.com/FabriceSalvaire/mupdf-cmake/blob/cmake/cmake/modules/FindOpenJPEG2.cmake
####################################################################################################
#
# - Find OPENJPEG2
# Find the native OPENJPEG2 includes and library
# This module defines
#  OPENJPEG2_INCLUDE_DIR, where to find openjpeg.h.
#  OPENJPEG2_LIBRARIES, the libraries needed to use OPENJPEG2.
#  OPENJPEG2_FOUND, If false, do not try to use OPENJPEG2.
# also defined, but not for general use are
#  OPENJPEG2_LIBRARY, where to find the OPENJPEG2 library.
#
# Written by Fabrice Salvaire
#
####################################################################################################

# UseOPENJPEG.cmake ?

# cf. FindPkgConfig: a pkg-config module for CMake

# find_path(OPENJPEG2_INCLUDE_DIR openjpeg.h)
exec_program("pkg-config"
  ARGS "--cflags-only-I libopenjp2"
  OUTPUT_VARIABLE OPENJPEG2_INCLUDE_DIR )
string(REGEX REPLACE "^-I" "" OPENJPEG2_INCLUDE_DIR ${OPENJPEG2_INCLUDE_DIR})

# set(OPENJPEG2_NAMES ${OPENJPEG2_NAMES} openjp2)
# find_library(OPENJPEG2_LIBRARY NAMES ${OPENJPEG2_NAMES})
exec_program("pkg-config"
  ARGS "--libs libopenjp2"
  OUTPUT_VARIABLE OPENJPEG2_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set OPENJPEG2_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENJPEG2 DEFAULT_MSG OPENJPEG2_LIBRARY OPENJPEG2_INCLUDE_DIR)

if(OPENJPEG2_FOUND)
  set(OPENJPEG2_LIBRARIES ${OPENJPEG2_LIBRARY})
endif()

mark_as_advanced(OPENJPEG2_LIBRARY OPENJPEG2_INCLUDE_DIR)

####################################################################################################
#
# End
#
####################################################################################################
