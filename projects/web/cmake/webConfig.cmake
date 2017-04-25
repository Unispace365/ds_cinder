# Hook into CEF's CMake files
# Not sure if this is the right way to do this...
# TODO: Use ENV variable, or something...?

# Set CEF_ROOT
if( NOT CEF_ROOT )
	get_filename_component( CEF_ROOT_ABS "${DS_CINDER_PATH}/../cef" ABSOLUTE )
	set( CEF_ROOT ${CEF_ROOT_ABS} CACHE STRING
		"Path to the binary CEF directory."
		FORCE
	)
	ds_log_w( "CEF_ROOT not specified, defaulting to: ${CEF_ROOT}" )
endif()
message( "CEF ROOT: ${CEF_ROOT}" )
list( APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake" )

find_package( CEF REQUIRED )

if( NOT TARGET web )
	get_filename_component( WEB_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../cef/src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND WEB_SRC_FILES
		${WEB_SRC_PATH}/private/web_service.cpp
		${WEB_SRC_PATH}/ds/ui/sprite/web.cpp
		${WEB_SRC_PATH}/private/web_app.cc
		${WEB_SRC_PATH}/private/web_handler.cc
	)

	add_library( web ${WEB_SRC_FILES} )

	# Place compiled library in project's lib directory
	set_target_properties ( web PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( web PROPERTIES OUTPUT_NAME "web_d" )
	endif()

	target_include_directories( web PUBLIC "${WEB_SRC_PATH}" )
	target_include_directories( web SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )

	message( "\n-----------------------------------------------------------------------\n\n" )
	PRINT_CEF_CONFIG()
	message( "\n\n-----------------------------------------------------------------------\n" )

	add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

	# Override output location of libcef_dll_wrapper
	set( LIBCEF_DLL_WRAPPER_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../cef/lib_linux64/${CMAKE_BUILD_TYPE}" )
	set_target_properties( libcef_dll_wrapper PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${LIBCEF_DLL_WRAPPER_OUTPUT_DIRECTORY} )

	# This macro is defined by CEF, sets include directories and compile flags for library
	#SET_LIBRARY_TARGET_PROPERTIES(web)
	add_dependencies(web libcef_dll_wrapper)

	# Logical target used to link the libcef library.
	ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
	target_link_libraries( web PUBLIC libcef_lib ${CEF_STANDARD_LIBS})

	# HACK: We need to specify path to libcef_dll_wrapper.a instead of just
	# plain target name (libcef_dll_wrapper) because otherwise, the "web"
	# target would inherit all the same compile flags as libcef_dll_wrapper.a,
	# which we don't want 
	target_link_libraries( web PRIVATE ${LIBCEF_DLL_WRAPPER_OUTPUT_DIRECTORY}/libcef_dll_wrapper.a )
	#print_target_properties(web)

	# HMM... WTF?
	target_link_libraries( web PRIVATE libgcrypt.so.11 )
	target_include_directories( web SYSTEM BEFORE PUBLIC ${CEF_INCLUDE_PATH} )

	# GLFW required to get access to window handle
	find_package( GLFW3 REQUIRED )
	target_include_directories( web PRIVATE ${GLFW3_INCLUDE_PATH} )

	# pull in ds_cinder's exported configuration
	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( web PRIVATE ds-cinder-platform )

	# pull in cinder's exported configuration
	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		)
	endif()
	target_link_libraries( web PUBLIC cinder )

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( web PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( web )

endif()

