# Hook into CEF's CMake files
# Not sure if this is the right way to do this...
# TODO: Use ENV variable, or something...?

# Set CEF_ROOT
#[[
if( NOT CEF_ROOT )
	get_filename_component( CEF_ROOT_ABS "${DS_CINDER_PATH}/../cef" ABSOLUTE )
	set( CEF_ROOT ${CEF_ROOT_ABS} CACHE PATH
		"Path to the binary CEF directory."
		FORCE
	)
	ds_log_w( "CEF_ROOT not specified, defaulting to: ${CEF_ROOT}" )
endif()
message( "CEF ROOT: ${CEF_ROOT}" )
list( APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake" )
]]


if( NOT TARGET web )
	
	get_filename_component( WEB_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cef" ABSOLUTE )
	get_filename_component( WEB_SRC_PATH "${WEB_INCLUDE_PATH}/src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND WEB_SRC_FILES
		${WEB_SRC_PATH}/private/web_service.cpp
		${WEB_SRC_PATH}/ds/ui/sprite/web.cpp
		${WEB_SRC_PATH}/private/web_app.cc
		${WEB_SRC_PATH}/private/web_handler.cpp
	)

	add_library( web ${WEB_SRC_FILES} )
	target_compile_features(web PUBLIC cxx_std_17)
	target_compile_definitions(web PUBLIC UNICODE _UNICODE)
	set_target_properties(web PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# Place compiled library in project's lib directory
	set_target_properties ( web PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( web PROPERTIES OUTPUT_NAME "web_d" )
	endif()

	target_include_directories( web PUBLIC 
		"${WEB_INCLUDE_PATH}" 
		"${WEB_INCLUDE_PATH}/include"
		"${WEB_INCLUDE_PATH}/src"
		"${WEB_INCLUDE_PATH}/src/private"
	)
	
	target_include_directories( web SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		target_link_directories( web PUBLIC "${WEB_INCLUDE_PATH}/lib64/debug")
	else()
		target_link_directories( web PUBLIC "${WEB_INCLUDE_PATH}/lib64/release")
	endif()

	target_link_libraries( web PUBLIC 
		libcef.lib
		libcef_dll_wrapper.lib
	)

	# pull in ds_cinder's exported configuration
	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( web PRIVATE ds-cinder-platform )

	#cinder
	include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
	set( LIBCINDER_LIB_DIRECTORY "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}v${MSVC_TOOLSET_VERSION}")
	target_link_libraries( video PRIVATE "${LIBCINDER_LIB_DIRECTORY}/cinder.lib" )
	target_include_directories( video PRIVATE "${CINDER_PATH}/include" )


	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	set_target_properties( web PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	cotire( web )

endif()

