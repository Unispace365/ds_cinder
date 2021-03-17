if( NOT TARGET essentials )
	get_filename_component( ESSENTIALS_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND ESSENTIALS_SRC_FILES
		${ESSENTIALS_SRC_PATH}/ds/debug/automator/actions/base_action.cpp
		${ESSENTIALS_SRC_PATH}/ds/debug/automator/actions/callback_action.cpp
		${ESSENTIALS_SRC_PATH}/ds/debug/automator/actions/drag_action.cpp
		${ESSENTIALS_SRC_PATH}/ds/debug/automator/actions/multifinger_tap_action.cpp
		${ESSENTIALS_SRC_PATH}/ds/debug/automator/actions/tap_action.cpp
		${ESSENTIALS_SRC_PATH}/ds/debug/automator/automator.cpp
		${ESSENTIALS_SRC_PATH}/ds/network/helper/delayed_node_watcher.cpp
		${ESSENTIALS_SRC_PATH}/ds/network/https_client.cpp
		${ESSENTIALS_SRC_PATH}/ds/network/smtp_request.cpp
		${ESSENTIALS_SRC_PATH}/ds/query/content/generic_content_model.cpp
		${ESSENTIALS_SRC_PATH}/ds/query/content/xml_content_loader.cpp
		${ESSENTIALS_SRC_PATH}/ds/touch/delayed_momentum.cpp
		${ESSENTIALS_SRC_PATH}/ds/touch/five_finger_cluster.cpp
		${ESSENTIALS_SRC_PATH}/ds/touch/touch_debug.cpp
		${ESSENTIALS_SRC_PATH}/ds/touch/view_dragger.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/button/image_button.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/button/layout_button.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/button/sprite_button.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/control/control_slider.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/drawing/drawing_canvas.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/interface_xml/interface_xml_importer.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/interface_xml/stylesheet_parser.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/layout/smart_layout.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/menu/component/cluster_view.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/menu/component/menu_item.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/menu/touch_menu.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/scroll/centered_scroll_area.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/scroll/infinity_scroll_list.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/scroll/scroll_area.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/scroll/scroll_bar.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/scroll/scroll_list.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/soft_keyboard/entry_field.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/soft_keyboard/soft_keyboard.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/soft_keyboard/soft_keyboard_builder.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/soft_keyboard/soft_keyboard_button.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/soft_keyboard/soft_keyboard_defs.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/sprite/donut_arc.cpp
		${ESSENTIALS_SRC_PATH}/ds/ui/sprite/png_sequence_sprite.cpp
	)

	add_library( essentials ${ESSENTIALS_SRC_FILES} )

	# Place compiled library in project's lib directory
	set_target_properties ( essentials PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( essentials PROPERTIES OUTPUT_NAME "essentials_d" )
	endif()

	target_include_directories( essentials PUBLIC "${ESSENTIALS_SRC_PATH}" )
	target_include_directories( essentials SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )

	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( essentials PRIVATE ds-cinder-platform )

	# pull in cinder's exported configuration
	if(NOT WIN32)
		if( NOT TARGET cinder )
			include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
			find_package( cinder REQUIRED PATHS
				"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
			)
		endif()
		target_link_libraries( essentials PUBLIC cinder )
	else()
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		set( LIBCINDER_LIB_DIRECTORY "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}v${MSVC_TOOLSET_VERSION}")
		target_link_libraries( essentials PRIVATE "${LIBCINDER_LIB_DIRECTORY}/cinder.lib" )
		target_include_directories( essentials PRIVATE "${CINDER_PATH}/include" )
	endif()

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( essentials PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( essentials )

endif()

