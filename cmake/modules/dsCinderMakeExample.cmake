include( CMakeParseArguments )

## TEMP: Debug/Trace INCLUDE_DIRECTORIES....
#set(CMAKE_DEBUG_TARGET_PROPERTIES INCLUDE_DIRECTORIES)

function( ds_cinder_make_example )
	set( oneValueArgs APP_PATH DS_CINDER_PATH )
	set( multiValueArgs SOURCES INCLUDES LIBRARIES RESOURCES PROJECT_COMPONENTS )

	cmake_parse_arguments( ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	if( NOT ARG_APP_PATH )
		ds_log_e( "No APP_PATH specified!" )
	endif()

	if( NOT DS_CINDER_APP_TARGET )
		set( DS_CINDER_APP_TARGET "${PROJECT_NAME}" )
	endif()

	if( ARG_UNPARSED_ARGUMENTS )
		ds_log_w( "unhandled arguments: ${ARG_UNPARSED_ARGUMENTS}" )
	endif()

	include( "${ARG_DS_CINDER_PATH}/cmake/configure.cmake" )

	# ALSO: include CINDER configure.cmake...
	include( "${CINDER_PATH}/proj/cmake/configure.cmake" )

	# Place executable in app's bin directory
	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ARG_APP_PATH}/bin )

	ds_log_v( "APP_NAME: ${DS_CINDER_APP_TARGET}" )
	ds_log_v( "APP_PATH: ${ARG_APP_PATH}" )
	ds_log_v( "SOURCES: ${ARG_SOURCES}" )
	ds_log_v( "INCLUDES: ${ARG_INCLUDES}" )
	ds_log_v( "LIBRARIES: ${ARG_LIBRARIES}" )
	ds_log_v( "DS_CINDER_PATH: ${ARG_DS_CINDER_PATH}" )
	ds_log_v( "CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" )
	ds_log_v( "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}" )
	ds_log_v( "DS_CINDER_TARGET: ${DS_CINDER_TARGET}" )
	ds_log_v( "DS_CINDER_LIB_DIRECTORY: ${DS_CINDER_LIB_DIRECTORY}" )

	ds_log_v( "CINDER_PATH: ${CINDER_PATH}" )
	ds_log_v( "CINDER_LIB_DIRECTORY: ${CINDER_LIB_DIRECTORY}" )
	#ds_log_v( "CINDER BLOCKS: ${ARG_BLOCKS}" )

	# This ensures that the application will link with the correct version of Cinder
	# based on the current build type without the need to remove the entire build folder
	# when switching build type after an initial configuration. See PR #1518 for more info. 
	if( cinder_DIR )
		unset( cinder_DIR CACHE )
	endif()

	# pull in ds-cinder-platform's exported configuration
	if( NOT TARGET ds-cinder-platform )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${ARG_DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
		)
	endif()

	# pull in cinder's exported configuration
	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		)
	endif()

	# ensure the runtime output directory exists, in case we need to copy other files to it
	if( NOT EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		file( MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
	endif()

	if( CINDER_MAC )
		# set icon. TODO: make this overridable
		set( ICON_NAME "CinderApp.icns" )
		set( ICON_PATH "${CINDER_PATH}/samples/data/${ICON_NAME}" )

		# copy .icns to bundle's resources folder
		set_source_files_properties( ${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
		# copy any other resources specified by user
		set_source_files_properties( ${ARG_RESOURCES} PROPERTIES HEADER_FILE_ONLY ON MACOSX_PACKAGE_LOCATION Resources )
	elseif( CINDER_LINUX )
		if( ARG_RESOURCES )
			# copy resources to a folder next to the app names 'resources'. note the folder is flat, so duplicates will be overwritten
			get_filename_component( RESOURCES_DEST_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources" ABSOLUTE )
			message( "copying resources to: ${RESOURCES_DEST_PATH}" )
			if( EXISTS "${RESOURCES_DEST_PATH}" )
				message( "resources destination path exists, removing old first." )
			endif()

			file( COPY "${ARG_RESOURCES}" DESTINATION "${RESOURCES_DEST_PATH}" )

			unset( ARG_RESOURCES ) # Don't allow resources to be added to the executable on linux
		endif()
	elseif( CINDER_MSW )		
		if( MSVC )
			# Override the default /MD with /MT
			foreach( 
				flag_var
				CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO 
				CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO 
			)
				if( ${flag_var} MATCHES "/MD" )
					string( REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}" )
					set( "${flag_var}" "${${flag_var}}" PARENT_SCOPE )
				endif()
			endforeach()
			# Force synchronous PDB writes
			add_compile_options( /FS ) 
			# Force multiprocess compilation
			add_compile_options( /MP )
			# Add lib dirs
			cmake_policy( PUSH )
			cmake_policy( SET CMP0015 OLD )
			link_directories( "${CINDER_PATH}/lib/${CINDER_TARGET_SUBFOLDER}" )
			cmake_policy( POP )
		endif()
	endif()

	add_executable( ${DS_CINDER_APP_TARGET} MACOSX_BUNDLE WIN32 ${ARG_SOURCES} ${ICON_PATH} ${ARG_RESOURCES} )

	# Add "_debug" to Debug build executable name
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( ${DS_CINDER_APP_TARGET} PROPERTIES OUTPUT_NAME "${DS_CINDER_APP_TARGET}_debug" )
	endif()

	target_include_directories( ${DS_CINDER_APP_TARGET} PUBLIC ${APP_PATH}/src )
	target_include_directories( ${DS_CINDER_APP_TARGET} PUBLIC ${ARG_INCLUDES} )
	target_link_libraries( ${DS_CINDER_APP_TARGET} cinder ds-cinder-platform ${ARG_LIBRARIES} )

	# Cinder Blocks (TODO: make this smarter?)
	set ( CINDER_BLOCKS )
	#list( APPEND CINDER_BLOCKS OSC TUIO )
	foreach( block ${CINDER_BLOCKS} )
		set( blockName "${block}" )
		get_filename_component( blockModuleDir "${CINDER_PATH}/blocks/${block}/proj/cmake" ABSOLUTE )
		find_package( ${blockName} PATHS ${blockModuleDir} NO_DEFAULT_PATH )
		add_dependencies( ${DS_CINDER_APP_TARGET} ${blockName} )
		#list( APPEND DS_CINDER_LIBS_DEPENDS ${blockName} )
		target_link_libraries( ${DS_CINDER_APP_TARGET} ${blockName} )
	endforeach()

	add_dependencies( ${DS_CINDER_APP_TARGET} ds-cinder-platform )

	# Ignore Specific Default Libraries
	if( MSVC )
		set_target_properties( ${DS_CINDER_APP_TARGET} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:LIBCPMT" )
	endif()

	# DsCinder projects/ components
	foreach( projectComponent ${ARG_PROJECT_COMPONENTS} )
		message( "\n----------------------------------------------------" )
		message(   "  Configuring DsCinder sub-project: ${projectComponent}" )
		message(   "----------------------------------------------------" )
		get_filename_component( projectComponentModuleDir "${DS_CINDER_PATH}/projects/${projectComponent}/cmake" ABSOLUTE )
		set( projectComponentName "" )
		ds_log_i( "projectComponentModuleDir: ${projectComponentModuleDir}" )

		if( EXISTS ${projectComponentModuleDir} )
			get_filename_component( projectComponentName "${projectComponent}" NAME )
		elseif( EXISTS ${DS_CINDER_PATH}/projects/${projectComponent}/cmake )
			get_filename_component( projectComponentModuleDir "${DS_CINDER_PATH}/projects/${projectComponent}/cmake" ABSOLUTE )
			set( projectComponentName "${projectComponent}" )
		elseif( IS_DIRECTORY ${projectComponent} )
		  	get_filename_component( projectComponentName ${projectComponent} NAME )
			set( projectComponentModuleDir "${projectComponent}/cmake" )
    		else()
			message( ERROR " Could not find projectComponent: ${projectComponent}, checked in ds_cinder/projects and at path: ${projectComponentModuleDir}" )
		endif()

		if( projectComponentName )
			find_package( ${projectComponentName} PATHS ${projectComponentModuleDir} NO_DEFAULT_PATH )
			# First check if a target was defined. If so then includes and extra libraries will automatically be added to the app target.
			if( TARGET "${projectComponentName}" )
				add_dependencies( ${DS_CINDER_APP_TARGET} "${projectComponentName}" )
				target_link_libraries( ${DS_CINDER_APP_TARGET} "${projectComponentName}" )
			else()
				# Otherwise, check for either includes for a header-only projectComponent or libraries that need to be linked.
				# - sanity check to warn if someone passed in a projectComponent with unexpected cmake configuration
				if( NOT ${projectComponentName}_INCLUDES AND NOT ${projectComponentName}_LIBRARIES )
					message( WARNING "no target defined for '${projectComponentName}', so expected either ${projectComponentName}_INCLUDES} or ${projectComponentName}_LIBRARIES to be defined but neither were found." )
				endif()

				if( ${projectComponentName}_INCLUDES )
					target_include_directories( ${DS_CINDER_APP_TARGET} PUBLIC ${${projectComponentName}_INCLUDES} )
				endif()
				if( ${projectComponentName}_LIBRARIES )
					target_link_libraries( ${DS_CINDER_APP_TARGET} ${${projectComponentName}_LIBRARIES} )
				endif()
			endif()

			ds_log_v( "Added projectComponent named: ${projectComponentName}, module directory: ${projectComponentModuleDir}" )
		endif()
	endforeach()

	if( CINDER_MAC )
		# set bundle info.plist properties
		set_target_properties( ${DS_CINDER_APP_TARGET} PROPERTIES
			MACOSX_BUNDLE_BUNDLE_NAME ${DS_CINDER_APP_TARGET}
			MACOSX_BUNDLE_ICON_FILE ${ICON_NAME}
		)
	endif()

	# Make building wai faster using Cotire
	#include( cotire )
	#set_target_properties( ${DS_CINDER_APP_TARGET} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( ${DS_CINDER_APP_TARGET} )

endfunction()

