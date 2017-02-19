cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

# Project start
project( ds-cinder-platform )

get_filename_component( ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE )
set( DS_CINDER_PATH    		"${CMAKE_CURRENT_SOURCE_DIR}" )
set( DS_CINDER_CMAKE_DIR	"${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

# 1. Configure (configure.cmake), used by user-apps and Examples
#		Setup verbose option 
#		Setup BUILD_SHARED_LIBS lib option 
#		Setup build type (debug/release)
#		Setup target platform( winodws, linux, ios, android...)
#		Setup GL target (ogl, es2, es3, etc...)
#		Setup <project>_LIB_DIRECTORY (
#       	i.e. Cinder uses:
#				set( CINDER_LIB_DIRECTORY "lib/${<project>_TARGET_SUBFOLDER}/${CMAKE_BUILD_TYPE}/" )
# 			IMPORTANT: This is where user-apps expect lib to be found
include( ${DS_CINDER_CMAKE_DIR}/configure.cmake )

# 2. Configure (lib<project>_configure.cmake)
#		Setup <project>_SRC_DIR
#		Setup <project>_INC_DIR
#		Setup <project>_INCLUDE__(SYSTEM|USER)_(PRIVATE|INTERFACE) paths
include( ${DS_CINDER_CMAKE_DIR}/libds-cinder-platform_configure.cmake )

# 3. SOURCE FILES for <project> (lib<project>_source_files.cmake)
#		Setup a bunch of different source_groups...
#		Set DS_CINDER_SRC_FILES
include( ${DS_CINDER_CMAKE_DIR}/libds-cinder-platform_source_files.cmake )

# 4. Platform-specific cmake stuff ( libs, etc...) (platform_linux.cmake)
#		Mostly add platform(linux)-specific include dirs and libraries
if( DS_CINDER_LINUX )
	include( "${DS_CINDER_CMAKE_DIR}/platform_linux.cmake" )
elseif( DS_CINDER_MSW )
	include( "${DS_CINDER_CMAKE_DIR}/platform_msw.cmake" )
endif()

# 5. TARGET <project> (lib<project)_target.cmake)
include( ${DS_CINDER_CMAKE_DIR}/libds-cinder-platform_target.cmake )

# 6. Build Examples?

# 7. Build Tests?

## Enable Warnings
#set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
## Disable a ton of Cinder initialization order warning
#set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder" )
## Enable C++14
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
##SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# Setup include directories
#set( INC_DIRS
#${ROOT_PATH}/src
#${ROOT_PATH}/lib/poco/include
#${CINDER_PATH}/include
#)
#include_directories( ${INC_DIRS} )

# Cinder Blocks
##include ( cinderBlocks ) 
##load_cinder_blocks( BLOCKS TUIO CINDER_PATH ${CINDER_PATH} )
#include_directories( ${CINDER_PATH}/blocks/TUIO/src/cinder )
#include_directories( ${CINDER_PATH}/blocks/OSC/src )

# Libs

# Pango
#find_package(Pango REQUIRED)
#include_directories(${Pango_1_0_INCLUDE_DIR})
#target_link_libraries (ds-cinder-platform ${BZIP_INCLUDE_DIRS})

# Glib
#find_package(Glib REQUIRED)
#include_directories(${Glib_INCLUDE_DIRS})

# Cairo
#find_package(Cairo REQUIRED)
#include_directories(${CAIRO_INCLUDE_DIRS})

#add_library( ${PROJECT_NAME}    ${SRC_FILES} )

# Make building wai faster using Cotire
#include(cotire)
#set_target_properties( ds-cinder-platform PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
#cotire(ds-cinder-platform)

