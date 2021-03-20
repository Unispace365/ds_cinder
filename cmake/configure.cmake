option( DS_CINDER_VERBOSE "Print verbose build configuration messages. " OFF )
include( ${CMAKE_CURRENT_LIST_DIR}/utilities.cmake )

# Set default build type to Debug
if( NOT CMAKE_BUILD_TYPE )
	ds_log_v( "CMAKE_BUILD_TYPE not specified, defaulting to Debug" )
	set( CMAKE_BUILD_TYPE Debug CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel. "
		FORCE
	)
endif()

ds_log_v( "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}" )

set( DS_CINDER_TARGET "Microsoft Windows" ) #This is used for friendly name output.
set( DS_CINDER_MSW TRUE )
set( DS_CINDER_ARCH "x86" ) #do we even support x86?
if( CMAKE_CL_64 )
	set( DS_CINDER_ARCH "x64" )
endif()
set( DS_CINDER_TARGET_SUBFOLDER "msw/${DS_CINDER_ARCH}/" ) # TODO: place in msw/arch folder (x64 or x86)

# DS_CINDER_LIB_DIRECTORY is the platform-specific, relative path that will be used to define
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY for libds-cinder-platform and also specifies where user apps will locate the cinder package
#set( DS_CINDER_LIB_DIRECTORY "lib/${DS_CINDER_TARGET_SUBFOLDER}/${CMAKE_BUILD_TYPE}/" )
 
# Instead of following CINDER convention, let's put all the DsCinder libs in lib/ds_cinder/(Debug|Release)
#[[
AF-2021-3-19:the original value here is

set( DS_CINDER_LIB_DIRECTORY "lib/ds_cinder/${CMAKE_BUILD_TYPE}" )

This puts the ds-cinder-platform.lib in a folder with source libraries and makes 
for awkward .gitignore and cleans. changing it to be a subdirectory of "out".
]]
file(RELATIVE_PATH _relative_out ${DS_CINDER_PATH} ${CMAKE_BINARY_DIR})
message("relative out: ${_relative_out}")
set( DS_CINDER_LIB_DIRECTORY "lib/ds_cinder" )

# Setup CINDER_PATH
# TODO allow this to be set from CMAKE cache settings settings
get_filename_component( CINDER_PATH "${DS_CINDER_PATH}/Cinder" ABSOLUTE )
