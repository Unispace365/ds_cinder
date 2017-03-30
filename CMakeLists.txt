cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
#set( CMAKE_VERBOSE_MAKEFILE ON )

# Project start
project( ds-cinder-platform )

get_filename_component( ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE )
set( DS_CINDER_PATH    		"${CMAKE_CURRENT_SOURCE_DIR}" )
set( DS_CINDER_CMAKE_DIR	"${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

option( DS_CINDER_BUILD_EXAMPLES "Build all examples." OFF )

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
if( DS_CINDER_BUILD_EXAMPLES )
	include( ${DS_CINDER_CMAKE_DIR}/modules/findCMakeDirs.cmake )

	set( allExamples "" )
	findCMakeDirs( allExamples "${DS_CINDER_CMAKE_DIR}/examples" "${DS_CINDER_SKIP_EXAMPLES}" )
	foreach( exampleDir ${allExamples} )
		ds_log_i( "adding example: ${exampleDir}" )
		add_subdirectory( ${exampleDir} )
	endforeach()
endif()


# 7. Build Tests?
