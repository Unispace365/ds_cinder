if( NOT TARGET viewers )
	get_filename_component( VIEWERS_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND VIEWERS_SRC_FILES

		${VIEWERS_SRC_PATH}/ds/ui/media/media_interface.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/media_interface.h
		${VIEWERS_SRC_PATH}/ds/ui/media/media_interface_builder.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/media_interface_builder.h
		${VIEWERS_SRC_PATH}/ds/ui/media/media_player.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/media_player.h
		${VIEWERS_SRC_PATH}/ds/ui/media/media_slideshow.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/media_slideshow.h
		${VIEWERS_SRC_PATH}/ds/ui/media/media_viewer.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/media_viewer.h
		${VIEWERS_SRC_PATH}/ds/ui/media/media_viewer_settings.h
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/pdf_interface.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/pdf_interface.h
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/thumbnail_bar.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/thumbnail_bar.h
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/video_interface.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/video_interface.h
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/video_scrub_bar.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/video_scrub_bar.h
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/video_volume_control.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/video_volume_control.h
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/web_interface.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/web_interface.h
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/youtube_interface.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/interface/youtube_interface.h
		${VIEWERS_SRC_PATH}/ds/ui/media/player/panoramic_video_player.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/player/panoramic_video_player.h
		${VIEWERS_SRC_PATH}/ds/ui/media/player/pdf_player.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/player/pdf_player.h
		${VIEWERS_SRC_PATH}/ds/ui/media/player/stream_player.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/player/stream_player.h
		${VIEWERS_SRC_PATH}/ds/ui/media/player/video_player.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/player/video_player.h
		${VIEWERS_SRC_PATH}/ds/ui/media/player/web_player.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/player/web_player.h
		${VIEWERS_SRC_PATH}/ds/ui/media/player/youtube_player.cpp
		${VIEWERS_SRC_PATH}/ds/ui/media/player/youtube_player.h
		${VIEWERS_SRC_PATH}/ds/ui/panel/base_panel.cpp
		${VIEWERS_SRC_PATH}/ds/ui/panel/base_panel.h
		${VIEWERS_SRC_PATH}/ds/ui/panel/panel_layouts.cpp
		${VIEWERS_SRC_PATH}/ds/ui/panel/panel_layouts.h
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/algoGil.cpp
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/algoGil.h
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/algoMaxRects.cpp
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/algoMaxRects.h
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/algoShelfSimple.cpp
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/algoShelfSimple.h
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/pixelAlgo.h
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/pixelPak.cpp
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/pak/pixelPak.h
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/util/myBox.cpp
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/util/myBox.h
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/util/myVector2.cpp
		${VIEWERS_SRC_PATH}/ds/util/pixel_packer/util/myVector2.h

	)

	add_library( viewers ${VIEWERS_SRC_FILES} )
	target_compile_features(viewers PUBLIC cxx_std_17)
	target_compile_definitions(viewers PUBLIC UNICODE _UNICODE)
	set_target_properties(viewers PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# Place compiled library in project's lib directory
	set_target_properties ( viewers PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( viewers PROPERTIES OUTPUT_NAME "viewers_d" )
	endif()

	target_include_directories( viewers PUBLIC "${VIEWERS_SRC_PATH}" )
	target_include_directories( viewers SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )

	# Dependent components
	list( APPEND PROJECT_COMPONENT_DEPS  video web pdf )
	foreach( projectName ${PROJECT_COMPONENT_DEPS} )
		#get_filename_component( projectComponentModuleDir "${DS_CINDER_PATH}/projects/${projectName}/cmake" ABSOLUTE )
		get_filename_component( projectComponentModuleDir "${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" ABSOLUTE )
		ds_log_i("Module dir: ${projectComponentModuleDir}")
		find_package( ${projectName} PATHS ${projectComponentModuleDir} NO_DEFAULT_PATH )
		add_dependencies( viewers "${projectName}" )
		target_link_libraries( viewers PUBLIC "${projectName}" )
	endforeach()

	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" 
		)
	endif()
	target_link_libraries( viewers PUBLIC ds-cinder-platform )

	#cinder
	include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
	set( LIBCINDER_LIB_DIRECTORY "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}v${MSVC_TOOLSET_VERSION}")
	target_link_libraries( mosquitto PRIVATE "${LIBCINDER_LIB_DIRECTORY}/cinder.lib" )
	target_include_directories( mosquitto PRIVATE "${CINDER_PATH}/include" )

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( viewers PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( viewers )

endif()



