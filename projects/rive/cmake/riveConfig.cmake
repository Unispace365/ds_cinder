if( NOT TARGET ds_rive )
	get_filename_component( DS_RIVE_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../src" ABSOLUTE )
	get_filename_component( RIVE_RUNTIME_PATH "${CMAKE_CURRENT_LIST_DIR}/../rive-runtime" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND RIVE_SRC_FILES
		${DS_RIVE_SRC_PATH}/rive/RiveSprite.cpp
	)

	add_library( ds_rive ${RIVE_SRC_FILES} )

	# Place compiled library in project's lib directory
	set_target_properties ( ds_rive PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( ds_rive PROPERTIES OUTPUT_NAME "ds_rive_d" )
	endif()

	target_include_directories( ds_rive PUBLIC "${RIVE_RUNTIME_PATH}/include" )
	target_include_directories( ds_rive PUBLIC "${DS_RIVE_SRC_PATH}" )
	target_include_directories( ds_rive SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )

	# Rive
	find_package( Rive REQUIRED )
	target_include_directories( ds_rive SYSTEM BEFORE PRIVATE ${Rive_INCLUDE_DIRS} )
	target_link_libraries( ds_rive PRIVATE ${Rive_LIBRARIES} )
	
	# pull in cinder's exported configuration
	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		)
	endif()
	target_link_libraries( ds_rive PUBLIC cinder )

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( ds_rive PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( ds_rive )

endif()



