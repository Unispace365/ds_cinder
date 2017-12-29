if( NOT TARGET video )
	get_filename_component( VIDEO_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../gstreamer-1.0/src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND VIDEO_SRC_FILES
		${VIDEO_SRC_PATH}/gstreamer/gstreamer_wrapper.cpp
		${VIDEO_SRC_PATH}/gstreamer/gstreamer_env_check.cpp
		${VIDEO_SRC_PATH}/gstreamer/video_meta_cache.cpp
		${VIDEO_SRC_PATH}/private/gst_video_service.cpp
		${VIDEO_SRC_PATH}/ds/ui/sprite/panoramic_video.cpp
		${VIDEO_SRC_PATH}/ds/ui/sprite/gst_video.cpp
	)

	add_library( video ${VIDEO_SRC_FILES} )

	# Place compiled library in project's lib directory
	set_target_properties ( video PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( video PROPERTIES OUTPUT_NAME "video_d" )
	endif()

	target_include_directories( video PUBLIC "${VIDEO_SRC_PATH}" )
	target_include_directories( video SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )

	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( video PRIVATE ds-cinder-platform )

	# pull in cinder's exported configuration
	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		)
	endif()
	target_link_libraries( video PUBLIC cinder )


	# GStreamer and its dependencies.
	# GStreamer
	find_package( GStreamer REQUIRED COMPONENTS net)
	target_link_libraries( video PUBLIC
		${GSTREAMER_LIBRARIES}
		${GSTREAMER_BASE_LIBRARIES}
		${GSTREAMER_APP_LIBRARIES}
		${GSTREAMER_VIDEO_LIBRARIES}
		${GSTREAMER_NET_LIBRARIES}
	)
	target_include_directories( video SYSTEM BEFORE PRIVATE 
		${GSTREAMER_INCLUDE_DIRS}
		${GSTREAMER_BASE_INCLUDE_DIRS}
		${GSTREAMER_APP_INCLUDE_DIRS}
		${GSTREAMER_VIDEO_INCLUDE_DIRS}
		${GSTREAMER_NET_INCLUDE_DIRS}
	)

	# MediaInfo
	find_package( MediaInfoLib REQUIRED )
	target_link_libraries( video PUBLIC ${MEDIAINFOLIB_LIBRARIES} )
	target_include_directories( video SYSTEM BEFORE PRIVATE ${MEDIAINFOLIB_INCLUDE_DIRS})

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( video PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( video )

endif()


