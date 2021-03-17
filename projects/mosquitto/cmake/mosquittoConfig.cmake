if( NOT TARGET mosquitto )
	get_filename_component( MOSQUITTO_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../src" ABSOLUTE )
	get_filename_component( MOSQUITTO_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/../lib64" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND MOSQUITTO_SRC_FILES
		${MOSQUITTO_SRC_PATH}/ds/network/mqtt/mqtt_watcher.cpp
	)
	add_library( mosquitto ${MOSQUITTO_SRC_FILES} )
	target_compile_features(mosquitto PUBLIC cxx_std_17)
	target_compile_definitions(mosquitto PUBLIC UNICODE _UNICODE)
	set_target_properties(mosquitto PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# Place compiled library in project's lib directory
	set_target_properties ( mosquitto PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( mosquitto PROPERTIES OUTPUT_NAME "mosquitto_d" )
	endif()

	#libraries
	target_link_directories( mosquitto PUBLIC ${MOSQUITTO_LIB_PATH})
	target_link_libraries( mosquitto PUBLIC mosquitto.lib )
	target_link_libraries( mosquitto PUBLIC mosquittopp.lib )

	#includes
	target_include_directories( mosquitto PUBLIC "${MOSQUITTO_SRC_PATH}" )
	target_include_directories( mosquitto SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )
	target_include_directories( mosquitto SYSTEM PRIVATE "${MOSQUITTO_SRC_PATH}/ds/network/mosquitto" )


	# pull in ds_cinder's exported configuration
	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( mosquitto PRIVATE ds-cinder-platform )


	# pull in cinder's exported configuration
	
	set( LIBCINDER_LIB_DIRECTORY "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}v${MSVC_TOOLSET_VERSION}")
	target_link_libraries( mosquitto PRIVATE "${LIBCINDER_LIB_DIRECTORY}/cinder.lib" )
	target_include_directories( mosquitto PRIVATE "${CINDER_PATH}/include" )
	

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( mosquitto PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( mosquitto )

endif()


