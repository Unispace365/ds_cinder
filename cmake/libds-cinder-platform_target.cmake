cmake_minimum_required( VERSION 3.18 FATAL_ERROR )

# Has something to do with  INTERFACE_LINK_LIBRARIES (NEW)
# vs.
# (IMPORTED_)?LINK_INTERFACE_LIBRARIES(_<CONFIG>) (Old)?vs  
cmake_policy( SET CMP0022 NEW )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY} )

ds_log_v( "CMAKE_ARCHIVE_OUTPUT_DIRECTORY: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}" )

# The type is based on the value of the BUILD_SHARED_LIBS variable.
# When OFF ( default value ) Cinder will be built as a static lib
# and when ON as a shared library.
# See https://cmake.org/cmake/help/v3.0/command/add_library.html for more info.
add_library(
	ds-cinder-platform
	${DS_CINDER_SRC_FILES}
)
set_target_properties(ds-cinder-platform PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_compile_features(ds-cinder-platform PUBLIC cxx_std_17)
target_compile_definitions(ds-cinder-platform PUBLIC UNICODE _UNICODE)

# NOTE: These two lines aren't in the libcinder_target.cmake:
target_include_directories( ds-cinder-platform BEFORE PUBLIC ${DS_CINDER_INCLUDE_USER_PUBLIC} )
target_include_directories( ds-cinder-platform SYSTEM BEFORE PUBLIC ${DS_CINDER_INCLUDE_SYSTEM_PUBLIC} )

target_include_directories( ds-cinder-platform BEFORE INTERFACE ${DS_CINDER_INCLUDE_USER_INTERFACE} )
target_include_directories( ds-cinder-platform SYSTEM BEFORE INTERFACE ${DS_CINDER_INCLUDE_SYSTEM_INTERFACE} )

target_include_directories( ds-cinder-platform BEFORE PRIVATE ${DS_CINDER_INCLUDE_USER_PRIVATE} )
target_include_directories( ds-cinder-platform SYSTEM BEFORE PRIVATE ${DS_CINDER_INCLUDE_SYSTEM_PRIVATE} )

target_link_libraries( ds-cinder-platform PUBLIC ${DS_CINDER_LIBS_DEPENDS}  )

target_compile_definitions( ds-cinder-platform PUBLIC ${DS_CINDER_DEFINES} )


# Enable UNICODE
set( DS_CINDER_CXX_FLAGS "-DUNICODE -DNOMINMAX" )



# TODO: it would be nice to the following, but we can't until min required cmake is 3.3
#target_compile_options( ds-cinder-platform PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${DS_CINDER_CXX_FLAGS}> )
set( CMAKE_CXX_FLAGS "${DS_CINDER_CXX_FLAGS} ${CMAKE_CXX_FLAGS}" )
target_compile_options( ds-cinder-platform INTERFACE ${DS_CINDER_CXX_FLAGS} )

# This file will contain all dependencies, includes, definition, compiler flags and so on..
export( TARGETS ds-cinder-platform FILE ${PROJECT_BINARY_DIR}/${DS_CINDER_LIB_DIRECTORY}/ds-cinder-platformTargets.cmake )

# Register ds-cinder-platform with cmake user package registry
export( PACKAGE ds-cinder-platform )

# And this command will generate a file on the ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
# that applications have to pull in order to link successfully with Cinder and its dependencies.
# This specific ds-cinder-platformConfig.cmake file will just hold a path to the above mention 
# ds-cinder-platformTargets.cmake file which holds the actual info.
configure_file( ${CMAKE_CURRENT_LIST_DIR}/modules/ds-cinder-platformConfig.buildtree.cmake.in
    "${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}/ds-cinder-platformConfig.cmake"
)

# Make building wai faster using Cotire
include( cotire )
set_target_properties( ds-cinder-platform PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
set_target_properties( ds-cinder-platform PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE )
cotire(ds-cinder-platform)

