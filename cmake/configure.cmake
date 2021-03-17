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
if(UNIX) 
	set( DS_CINDER_TARGET "linux" )
	set( DS_CINDER_LINUX TRUE )

	
	execute_process( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE DS_CINDER_ARCH )
	set( DS_CINDER_TARGET_SUBFOLDER "linux/${DS_CINDER_ARCH}" )

elseif(WIN32)
	set( DS_CINDER_TARGET "Microsoft Windows" )
	set( DS_CINDER_MSW TRUE )
	set( DS_CINDER_ARCH "x86" )
	if( CMAKE_CL_64 )
		set( DS_CINDER_ARCH "x64" )
	endif()
	set( DS_CINDER_TARGET_SUBFOLDER "msw/${DS_CINDER_ARCH}/" ) # TODO: place in msw/arch folder (x64 or x86)
endif()
# DS_CINDER_LIB_DIRECTORY is the platform-specific, relative path that will be used to define
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY for libds-cinder-platform and also specifies where user apps will locate the cinder package
#set( DS_CINDER_LIB_DIRECTORY "lib/${DS_CINDER_TARGET_SUBFOLDER}/${CMAKE_BUILD_TYPE}/" )

# Instead of following CINDER convention, let's put all the DsCinder libs in lib/ds_cinder/(Debug|Release)
set( DS_CINDER_LIB_DIRECTORY "lib/ds_cinder/${CMAKE_BUILD_TYPE}" )

# Setup CINDER_PATH
#	TODO: Make this based on ENV var? 
get_filename_component( CINDER_PATH "${DS_CINDER_PATH}/cinder" ABSOLUTE )
