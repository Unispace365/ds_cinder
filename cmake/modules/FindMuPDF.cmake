# - Try to find MuPDF
# Once done, this will define:
#
#  MuPDF_FOUND - system has MuPDF
#  MuPDF_INCLUDE_DIRS - the MuPDF include directories
#  MuPDF_LIBRARIES - link these to use MuPDF

include(LibFindMacros)

# MuPDF Dependencies
#libfind_package( MuPDF JPEG )
find_package( JPEG )
libfind_package( MuPDF OpenJPEG2 )
libfind_package( MuPDF jbig2dec )

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(MUPDF_PKGCONF MuPDF)

# Include dir
find_path(MuPDF_INCLUDE_DIR
  NAMES mupdf/pdf.h
  HINTS ${MUPDF_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(MuPDF_LIBRARY
  NAMES mupdf
  PATHS ${MUPDF_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
set(MuPDF_PROCESS_INCLUDES MuPDF_INCLUDE_DIR
	OPENJPEG2_INCLUDE_DIR
	JBIG2DEC2_INCLUDE_DIR
)
set(MuPDF_PROCESS_LIBS MuPDF_LIBRARY
	JPEG_LIBRARY
	OPENJPEG2_LIBRARY
	JBIG2DEC_LIBRARY
)
libfind_process(MuPDF)
