if( NOT TARGET nvpath )
	get_filename_component( NVPATH_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND NVPATH_SRC_FILES
		${NVPATH_SRC_PATH}/nvpath/NvPath.cpp
		${NVPATH_SRC_PATH}/nvpath/NvPathSvgcpp
	)

	add_library( nvpath ${NVPATH_SRC_FILES} )

	# Place compiled library in project's lib directory
	set_target_properties ( nvpath PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( nvpath PROPERTIES OUTPUT_NAME "nvpath_d" )
	endif()

	target_include_directories( nvpath PUBLIC "${NVPATH_SRC_PATH}" )
	target_include_directories( nvpath SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )
	
	# pull in cinder's exported configuration
	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		)
	endif()
	target_link_libraries( nvpath PUBLIC cinder )

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( nvpath PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( nvpath )

endif()



