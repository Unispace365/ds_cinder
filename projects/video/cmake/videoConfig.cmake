if( NOT TARGET video )
	
	get_filename_component( VIDEO_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../gstreamer-1.0/src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND VIDEO_SRC_FILES
		ds/ui/sprite/gst_video.cpp
		ds/ui/sprite/gst_video.h
		ds/ui/sprite/panoramic_video.cpp
		ds/ui/sprite/panoramic_video.h
		ds/ui/sprite/video.h
		gstreamer/gstreamer_audio_device.cpp
		gstreamer/gstreamer_audio_device.h
		gstreamer/gstreamer_env_check.cpp
		gstreamer/gstreamer_env_check.h
		gstreamer/gstreamer_wrapper.cpp
		gstreamer/gstreamer_wrapper.h
		gstreamer/video_meta_cache.cpp
		gstreamer/video_meta_cache.h
		private/gst_video_service.cpp
		private/gst_video_service.h
	)
	list (TRANSFORM VIDEO_SRC_FILES PREPEND ${VIDEO_SRC_PATH}/)

	add_library( video ${VIDEO_SRC_FILES} )
	target_compile_features(video PUBLIC cxx_std_17)
	target_compile_definitions(video PUBLIC UNICODE _UNICODE)
	set_target_properties(video PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# Place compiled library in project's lib directory
	set_target_properties ( video PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( video PROPERTIES OUTPUT_NAME "video_d" )
	endif()

	target_include_directories( video PUBLIC "${VIDEO_SRC_PATH}" )
	target_include_directories( video PUBLIC "${VIDEO_SRC_PATH}/.." )
	target_include_directories( video SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )

	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( video PRIVATE ds-cinder-platform )

	#cinder
	include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
	set( LIBCINDER_LIB_DIRECTORY "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}v${MSVC_TOOLSET_VERSION}")
	target_link_libraries( video PRIVATE "${LIBCINDER_LIB_DIRECTORY}/cinder.lib" )
	target_include_directories( video PRIVATE "${CINDER_PATH}/include" )



	# GStreamer and its dependencies.
	# GStreamer
	set( GSTREAMER_ROOT_PATH $ENV{GSTREAMER_1_0_ROOT_X86_64} CACHE PATH "GStreamer Root Path" )
	ds_log_i("GSTREAMER_ROOT_PATH ${GSTREAMER_ROOT_PATH}")
	target_include_directories ( video PUBLIC 
		${GSTREAMER_ROOT_PATH}/include/gstreamer-1.0
		${GSTREAMER_ROOT_PATH}/lib/glib-2.0/include
		${GSTREAMER_ROOT_PATH}/include/glib-2.0
		${GSTREAMER_ROOT_PATH}/include
		${GSTREAMER_ROOT_PATH}/lib/gstreamer-1.0/include
	)

	target_link_libraries( video PUBLIC
		dsound.lib
		${GSTREAMER_ROOT_PATH}/lib/gstreamer-1.0.lib
		${GSTREAMER_ROOT_PATH}/lib/glib-2.0.lib
		${GSTREAMER_ROOT_PATH}/lib/gobject-2.0.lib
		${GSTREAMER_ROOT_PATH}/lib/gstapp-1.0.lib
		${GSTREAMER_ROOT_PATH}/lib/gstbase-1.0.lib
		${GSTREAMER_ROOT_PATH}/lib/gstvideo-1.0.lib
		${GSTREAMER_ROOT_PATH}/lib/gthread-2.0.lib
		${GSTREAMER_ROOT_PATH}/lib/gstnet-1.0.lib
		${GSTREAMER_ROOT_PATH}/lib/gstgl-1.0.lib
		#${GSTREAMER_ROOT_PATH}/lib/video_gstreamer-1.0_d.lib
	)

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( video PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( video )

endif()


