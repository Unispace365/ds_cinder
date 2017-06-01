# From https://github.com/lemire/FastPFor/blob/master/cmake_modules/Findsnappy.cmake
# Snappy, a fast compressor/decompressor
# Once done, this will define
#
#  Snappy_FOUND - system has Glib
#  Snappy_INCLUDE_DIRS - the Glib include directories
#  Snappy_LIBRARIES - link these to use Glib

include(LibFindMacros)

find_path(Snappy_INCLUDE_DIR
	NAMES snappy.h
)

find_library(Snappy_LIBRARY
	NAMES snappy
)

set(Snappy_PROCESS_INCLUDES Snappy_INCLUDE_DIR)
set(Snappy_PROCESS_LIBS Snappy_LIBRARY)

libfind_process(Snappy)
