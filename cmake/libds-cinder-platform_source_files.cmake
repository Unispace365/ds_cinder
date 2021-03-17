# This file is used to specify sources that become part of the libds-cinder-platform binary

#powershell get file list in directory: ((ls -Path . -Recurse *.* -Attributes !Directory | Resolve-Path -Relative) -replace "\.\\","") -replace "\\","/" | clip

list( APPEND DS_CINDER_DS_FILES 
	
	app/app.cpp
	app/app.h
	app/app_defs.cpp
	app/app_defs.h
	app/auto_draw.cpp
	app/auto_draw.h
	app/auto_update.cpp
	app/auto_update.h
	app/auto_update_list.cpp
	app/auto_update_list.h
	app/blob_reader.cpp
	app/blob_reader.h
	app/blob_registry.cpp
	app/blob_registry.h
	app/camera_utils.cpp
	app/camera_utils.h
	app/environment.cpp
	app/environment.h
	app/event.cpp
	app/event.h
	app/event_client.cpp
	app/event_client.h
	app/event_notifier.cpp
	app/event_notifier.h
	app/event_registry.cpp
	app/event_registry.h
	app/image_registry.cpp
	app/image_registry.h

	app/engine/engine.cpp
	app/engine/engine.h
	app/engine/engine_cfg.cpp
	app/engine/engine_cfg.h
	app/engine/engine_client.cpp
	app/engine/engine_client.h
	app/engine/engine_clientserver.cpp
	app/engine/engine_clientserver.h
	app/engine/engine_client_list.cpp
	app/engine/engine_client_list.h
	app/engine/engine_data.cpp
	app/engine/engine_data.h
	app/engine/engine_events.h
	app/engine/engine_io.cpp
	app/engine/engine_io.h
	app/engine/engine_io_defs.cpp
	app/engine/engine_io_defs.h
	app/engine/engine_roots.cpp
	app/engine/engine_roots.h
	app/engine/engine_server.cpp
	app/engine/engine_server.h
	app/engine/engine_service.h
	app/engine/engine_settings.cpp
	app/engine/engine_settings.h
	app/engine/engine_standalone.cpp
	app/engine/engine_standalone.h
	app/engine/engine_stats_view.cpp
	app/engine/engine_stats_view.h
	app/engine/engine_touch_queue.h
	app/engine/unique_id.cpp
	app/engine/unique_id.h
	
	cfg/settings.cpp
	cfg/settings.h
	cfg/settings_editor.cpp
	cfg/settings_editor.h
	cfg/settings_variables.cpp
	cfg/settings_variables.h

	cfg/editor_components/editor_item.cpp
	cfg/editor_components/editor_item.h
	cfg/editor_components/edit_view.cpp
	cfg/editor_components/edit_view.h

	content/content_events.h
	content/content_model.cpp
	content/content_model.h
	content/content_query.cpp
	content/content_query.h
	content/content_wrangler.cpp
	content/content_wrangler.h

	data/color_list.cpp
	data/color_list.h
	data/data_buffer.cpp
	data/data_buffer.h
	data/font_list.cpp
	data/font_list.h
	data/key_value_store.cpp
	data/key_value_store.h
	data/read_write_buffer.cpp
	data/read_write_buffer.h
	data/resource.cpp
	data/resource.h
	data/resource_list.cpp
	data/resource_list.h
	data/tuio_object.cpp
	data/tuio_object.h
	data/user_data.cpp
	data/user_data.h
	
	debug/apphost_stats_view.cpp
	debug/apphost_stats_view.h
	debug/auto_refresh.cpp
	debug/auto_refresh.h
	debug/computer_info.cpp
	debug/computer_info.h
	debug/console.h
	debug/debug_defines.cpp
	debug/debug_defines.h
	debug/function_exists.h
	debug/key_manager.cpp
	debug/key_manager.h
	debug/logger.cpp
	debug/logger.h
	
	debug/automator/automator.cpp
	debug/automator/automator.h
	debug/automator/actions/base_action.cpp
	debug/automator/actions/base_action.h
	debug/automator/actions/callback_action.cpp
	debug/automator/actions/callback_action.h
	debug/automator/actions/drag_action.cpp
	debug/automator/actions/drag_action.h
	debug/automator/actions/multifinger_tap_action.cpp
	debug/automator/actions/multifinger_tap_action.h
	debug/automator/actions/tap_action.cpp
	debug/automator/actions/tap_action.h
	
	gl/uniform.cpp
	gl/uniform.h
	
	math/fparser.cc
	math/fparser.hh
	math/fparser_gmpint.hh
	math/fparser_mpfr.hh
	math/fpconfig.hh
	math/fpoptimizer.cc
	math/math_defs.h
	math/math_func.cpp
	math/math_func.h
	
	math/extrasrc/fpaux.hh
	math/extrasrc/fptypes.hh
	math/extrasrc/fp_identifier_parser.inc
	math/extrasrc/fp_opcode_add.inc
	
	metrics/metrics_service.cpp
	metrics/metrics_service.h
	
	network/https_client.cpp
	network/https_client.h
	network/http_client.cpp
	network/http_client.h
	network/network_info.cpp
	network/network_info.h
	network/net_connection.h
	network/node_watcher.cpp
	network/node_watcher.h
	network/packet_chunker.cpp
	network/packet_chunker.h
	network/single_udp_receiver.cpp
	network/single_udp_receiver.h
	network/smtp_request.cpp
	network/smtp_request.h
	network/tcp_client.cpp
	network/tcp_client.h
	network/tcp_server.cpp
	network/tcp_server.h
	network/tcp_socket_sender.cpp
	network/tcp_socket_sender.h
	network/udp_connection.cpp
	network/udp_connection.h
	
	network/curl/curl.h
	network/curl/curlver.h
	network/curl/easy.h
	network/curl/mprintf.h
	network/curl/multi.h
	network/curl/stdcheaders.h
	network/curl/system.h
	network/curl/typecheck-gcc.h
	
	network/helper/delayed_node_watcher.cpp
	network/helper/delayed_node_watcher.h
	
	params/camera_params.cpp
	params/camera_params.h
	params/draw_params.h
	params/update_params.cpp
	params/update_params.h
	
	query/query_client.cpp
	query/query_client.h
	query/query_result.cpp
	query/query_result.h
	query/query_result_builder.cpp
	query/query_result_builder.h
	query/query_result_editor.cpp
	query/query_result_editor.h
	query/query_talkback.h
	query/recycle_array.h
	query/recycle_node.h
	query/sql_database.cpp
	query/sql_database.h
	query/sql_query_result_builder.cpp
	query/sql_query_result_builder.h
	
	query/content/generic_content_model.cpp
	query/content/generic_content_model.h
	query/content/xml_content_loader.cpp
	query/content/xml_content_loader.h
	
	query/sqlite/sqlite3.c
	query/sqlite/sqlite3.h
	query/sqlite/sqlite3ext.h
	
	service/weather_service.cpp
	service/weather_service.h
	
	storage/directory_watcher.cpp
	storage/directory_watcher.h
	storage/directory_watcher_win32.cpp
	storage/persistent_cache.cpp
	storage/persistent_cache.h
	
	thread/async_queue.h
	thread/gl_thread.cpp
	thread/gl_thread.h
	thread/parallel_runnable.h
	thread/runnable_client.cpp
	thread/runnable_client.h
	thread/serial_runnable.h
	thread/thread_defs.h
	thread/timed_runnable.h
	thread/work_client.cpp
	thread/work_client.h
	thread/work_manager.cpp
	thread/work_manager.h
	thread/work_request.cpp
	thread/work_request.h
	thread/work_request_list.h
	
	time/timer.cpp
	time/timer.h
	time/time_callback.cpp
	time/time_callback.h
	
	touch/delayed_momentum.cpp
	touch/delayed_momentum.h
	touch/five_finger_cluster.cpp
	touch/five_finger_cluster.h
	touch/view_dragger.cpp
	touch/view_dragger.h
	
	ui/button/image_button.cpp
	ui/button/image_button.h
	ui/button/layout_button.cpp
	ui/button/layout_button.h
	ui/button/sprite_button.cpp
	ui/button/sprite_button.h
	
	ui/control/control_check_box.cpp
	ui/control/control_check_box.h
	ui/control/control_slider.cpp
	ui/control/control_slider.h
	
	ui/drawing/drawing_canvas.cpp
	ui/drawing/drawing_canvas.h
	
	ui/interface_xml/interface_xml_importer.cpp
	ui/interface_xml/interface_xml_importer.h
	ui/interface_xml/stylesheet_parser.cpp
	ui/interface_xml/stylesheet_parser.h
	
	ui/layout/layout_sprite.cpp
	ui/layout/layout_sprite.h
	ui/layout/perspective_layout.cpp
	ui/layout/perspective_layout.h
	ui/layout/smart_layout.cpp
	ui/layout/smart_layout.h
	
	ui/menu/touch_menu.cpp
	ui/menu/touch_menu.h
	
	ui/menu/component/cluster_view.cpp
	ui/menu/component/cluster_view.h
	ui/menu/component/menu_item.cpp
	ui/menu/component/menu_item.h
	
	ui/scroll/centered_scroll_area.cpp
	ui/scroll/centered_scroll_area.h
	ui/scroll/infinity_scroll_list.cpp
	ui/scroll/infinity_scroll_list.h
	ui/scroll/scroll_area.cpp
	ui/scroll/scroll_area.h
	ui/scroll/scroll_bar.cpp
	ui/scroll/scroll_bar.h
	ui/scroll/scroll_list.cpp
	ui/scroll/scroll_list.h
	ui/scroll/smart_scroll_list.cpp
	ui/scroll/smart_scroll_list.h
	
	ui/service/load_image_service.cpp
	ui/service/load_image_service.h
	ui/service/pango_font_service.cpp
	ui/service/pango_font_service.h
	
	ui/soft_keyboard/entry_field.cpp
	ui/soft_keyboard/entry_field.h
	ui/soft_keyboard/soft_keyboard.cpp
	ui/soft_keyboard/soft_keyboard.h
	ui/soft_keyboard/soft_keyboard_builder.cpp
	ui/soft_keyboard/soft_keyboard_builder.h
	ui/soft_keyboard/soft_keyboard_button.cpp
	ui/soft_keyboard/soft_keyboard_button.h
	ui/soft_keyboard/soft_keyboard_defs.cpp
	ui/soft_keyboard/soft_keyboard_defs.h
	ui/soft_keyboard/soft_keyboard_settings.h
	
	ui/sprite/border.cpp
	ui/sprite/border.h
	ui/sprite/circle.cpp
	ui/sprite/circle.h
	ui/sprite/circle_border.cpp
	ui/sprite/circle_border.h
	ui/sprite/dashed_line.cpp
	ui/sprite/dashed_line.h
	ui/sprite/dirty_state.cpp
	ui/sprite/dirty_state.h
	ui/sprite/donut_arc.cpp
	ui/sprite/donut_arc.h
	ui/sprite/gradient_sprite.cpp
	ui/sprite/gradient_sprite.h
	ui/sprite/image.cpp
	ui/sprite/image.h
	ui/sprite/image_with_thumbnail.cpp
	ui/sprite/image_with_thumbnail.h
	ui/sprite/line.cpp
	ui/sprite/line.h
	ui/sprite/png_sequence_sprite.cpp
	ui/sprite/png_sequence_sprite.h
	ui/sprite/sprite.cpp
	ui/sprite/sprite.h
	ui/sprite/sprite_engine.cpp
	ui/sprite/sprite_engine.h
	ui/sprite/text.cpp
	ui/sprite/text.h
	ui/sprite/text_defs.cpp
	ui/sprite/text_defs.h
	
	ui/sprite/shader/sprite_shader.cpp
	ui/sprite/shader/sprite_shader.h
	
	ui/sprite/util/blend.cpp
	ui/sprite/util/blend.h
	ui/sprite/util/clip_plane.cpp
	ui/sprite/util/clip_plane.h
	ui/sprite/util/flexbox_parser.cpp
	ui/sprite/util/flexbox_parser.h
	
	ui/touch/button_behaviour.cpp
	ui/touch/button_behaviour.h
	ui/touch/drag_destination_info.h
	ui/touch/draw_touch_view.cpp
	ui/touch/draw_touch_view.h
	ui/touch/momentum.cpp
	ui/touch/momentum.h
	ui/touch/multi_touch_constraints.cpp
	ui/touch/multi_touch_constraints.h
	ui/touch/picking.cpp
	ui/touch/picking.h
	ui/touch/rotation_translator.cpp
	ui/touch/rotation_translator.h
	ui/touch/select_picking.cpp
	ui/touch/select_picking.h
	ui/touch/tap_info.h
	ui/touch/touch_debug.cpp
	ui/touch/touch_debug.h
	ui/touch/touch_event.h
	ui/touch/touch_info.h
	ui/touch/touch_manager.cpp
	ui/touch/touch_manager.h
	ui/touch/touch_mode.cpp
	ui/touch/touch_mode.h
	ui/touch/touch_process.cpp
	ui/touch/touch_process.h
	ui/touch/touch_translator.cpp
	ui/touch/touch_translator.h
	ui/touch/tuio_input.cpp
	ui/touch/tuio_input.h
	
	ui/tween/sprite_anim.cpp
	ui/tween/sprite_anim.h
	ui/tween/tweenline.cpp
	ui/tween/tweenline.h
	
	ui/util/sprite_cache.h
	ui/util/text_model.cpp
	ui/util/text_model.h
	ui/util/ui_utils.h
	
	util/bit_mask.cpp
	util/bit_mask.h
	util/color_util.cpp
	util/color_util.h
	util/date_util.cpp
	util/date_util.h
	util/exif.cpp
	util/exif.h
	util/exif_reader.h
	util/file_meta_data.cpp
	util/file_meta_data.h
	util/glm_structured_binding.h
	util/idle_timer.cpp
	util/idle_timer.h
	util/image_meta_data.cpp
	util/image_meta_data.h
	util/markdown_to_pango.cpp
	util/markdown_to_pango.h
	util/memory_ds.h
	util/notifier.h
	util/string_util.cpp
	util/string_util.h
	util/sundown/autolink.c
	util/sundown/autolink.h
	util/sundown/buffer.c
	util/sundown/buffer.h
	util/sundown/html_blocks.h
	util/sundown/markdown.c
	util/sundown/markdown.h
	util/sundown/stack.c
	util/sundown/stack.h
)
list (TRANSFORM DS_CINDER_DS_FILES PREPEND ${ROOT_PATH}/src/ds/)

list( APPEND DS_CINDER_OSC_FILES 
	OscArg.h
	OscBundle.cpp
	OscBundle.h
	OscListener.cpp
	OscListener.h
	OscMessage.cpp
	OscMessage.h
	OscSender.cpp
	OscSender.h
	ip/IpEndpointName.cpp
	ip/IpEndpointName.h
	ip/NetworkingUtils.h
	ip/PacketListener.h
	ip/TimerListener.h
	ip/UdpSocket.h
	osc/MessageMappingOscPacketListener.h
	osc/OscException.h
	osc/OscHostEndianness.h
	osc/OscOutboundPacketStream.cpp
	osc/OscOutboundPacketStream.h
	osc/OscPacketListener.h
	osc/OscPrintReceivedElements.cpp
	osc/OscPrintReceivedElements.h
	osc/OscReceivedElements.cpp
	osc/OscReceivedElements.h
	osc/OscTypes.cpp
	osc/OscTypes.h
)

if(NOT WIN32)
	list (APPEND DS_CINDER_OSC_FILES 
		ip/posix/NetworkingUtils.cpp
		ip/posix/UdpSocket.cpp
	)
else()
	list (APPEND DS_CINDER_OSC_FILES 
		ip/win32/NetworkingUtils.cpp
		ip/win32/UdpSocket.cpp
	)
endif()

list (TRANSFORM DS_CINDER_OSC_FILES PREPEND ${ROOT_PATH}/src/osc/)



list( APPEND DS_CINDER_TUIO_FILES 
	TuioClient.cpp
	TuioClient.h
	TuioCursor.h
	TuioObject.h
	TuioProfileBase.h
)
list (TRANSFORM DS_CINDER_TUIO_FILES PREPEND ${ROOT_PATH}/src/tuio/)


list( APPEND DS_CINDER_SRC_FILES 
	${ROOT_PATH}/src/stdafx.cpp
	${ROOT_PATH}/src/stdafx.h
	${DS_CINDER_DS_FILES} 
	${DS_CINDER_OSC_FILES}
	${DS_CINDER_TUIO_FILES}
	)

#[[
list( APPEND DS_CINDER_SRC_FILES
	${ROOT_PATH}/src/ds/app/engine/engine_io.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_server.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_client_list.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_roots.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_cfg.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_stats_view.cpp
	${ROOT_PATH}/src/ds/app/engine/unique_id.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_settings.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_data.cpp
	${ROOT_PATH}/src/ds/app/engine/engine.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_client.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_io_defs.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_standalone.cpp
	${ROOT_PATH}/src/ds/app/engine/engine_clientserver.cpp

	${ROOT_PATH}/src/stdafx.cpp
	${ROOT_PATH}/src/ds/app/event_client.cpp
	${ROOT_PATH}/src/ds/app/event_registry.cpp
	
	${ROOT_PATH}/src/ds/app/blob_reader.cpp
	${ROOT_PATH}/src/ds/app/camera_utils.cpp
	${ROOT_PATH}/src/ds/app/app_defs.cpp
	${ROOT_PATH}/src/ds/app/auto_update.cpp
	${ROOT_PATH}/src/ds/app/blob_registry.cpp
	${ROOT_PATH}/src/ds/app/app.cpp
	${ROOT_PATH}/src/ds/app/image_registry.cpp
	${ROOT_PATH}/src/ds/app/environment.cpp
	${ROOT_PATH}/src/ds/app/event.cpp
	${ROOT_PATH}/src/ds/app/auto_draw.cpp
	${ROOT_PATH}/src/ds/app/event_notifier.cpp
	${ROOT_PATH}/src/ds/app/auto_update_list.cpp

	${ROOT_PATH}/src/ds/util/date_util.cpp
	${ROOT_PATH}/src/ds/util/image_meta_data.cpp		# Need <intrin.h>
	${ROOT_PATH}/src/ds/util/file_meta_data.cpp
	${ROOT_PATH}/src/ds/util/color_util.cpp				# sprintf_s (windows)
	${ROOT_PATH}/src/ds/util/string_util.cpp
	${ROOT_PATH}/src/ds/util/idle_timer.cpp
	${ROOT_PATH}/src/ds/util/exif.cpp
	${ROOT_PATH}/src/ds/util/bit_mask.cpp

	${ROOT_PATH}/src/ds/gl/uniform.cpp
	${ROOT_PATH}/src/ds/network/http_client.cpp		# error: invalid initialization of non-const reference of type ‘std::unique_ptr<ds::WorkRequest>&’ from an rvalue of type ‘std::unique_ptr<ds::WorkRequest>’
	${ROOT_PATH}/src/ds/network/node_watcher.cpp
	${ROOT_PATH}/src/ds/network/packet_chunker.cpp
	${ROOT_PATH}/src/ds/network/tcp_client.cpp
	${ROOT_PATH}/src/ds/network/tcp_server.cpp
	${ROOT_PATH}/src/ds/network/tcp_socket_sender.cpp
	${ROOT_PATH}/src/ds/network/udp_connection.cpp
	${ROOT_PATH}/src/ds/network/single_udp_receiver.cpp
	${ROOT_PATH}/src/ds/network/network_info.cpp		# Uses winsock2 apis
	${ROOT_PATH}/src/ds/time/timer.cpp
	${ROOT_PATH}/src/ds/thread/work_request.cpp
	${ROOT_PATH}/src/ds/thread/work_manager.cpp
	${ROOT_PATH}/src/ds/thread/gl_thread.cpp			# Uses win32 and WGL APIs
	${ROOT_PATH}/src/ds/thread/runnable_client.cpp		# error: invalid initialization of non-const reference of type ‘std::unique_ptr<ds::WorkRequest>&’ from an rvalue of type ‘std::unique_ptr<ds::WorkRequest>’
	${ROOT_PATH}/src/ds/thread/work_client.cpp
	${ROOT_PATH}/src/ds/storage/directory_watcher_win32.cpp	# Uses win32 apis
	${ROOT_PATH}/src/ds/storage/persistent_cache.cpp
	${ROOT_PATH}/src/ds/storage/directory_watcher.cpp
	${ROOT_PATH}/src/ds/query/query_result.cpp			
	${ROOT_PATH}/src/ds/query/sql_query_result_builder.cpp
	${ROOT_PATH}/src/ds/query/query_result_builder.cpp
	${ROOT_PATH}/src/ds/query/sql_database.cpp
	${ROOT_PATH}/src/ds/query/query_client.cpp			# error: invalid initialization of non-const reference of type ‘std::unique_ptr<ds::WorkRequest>&’ from an rvalue of type ‘std::unique_ptr<ds::WorkRequest>’
	${ROOT_PATH}/src/ds/query/query_result_editor.cpp
	
	${ROOT_PATH}/src/ds/ui/touch/button_behaviour.cpp
	${ROOT_PATH}/src/ds/ui/touch/picking.cpp
	${ROOT_PATH}/src/ds/ui/touch/momentum.cpp
	${ROOT_PATH}/src/ds/ui/touch/touch_process.cpp
	${ROOT_PATH}/src/ds/ui/touch/multi_touch_constraints.cpp
	${ROOT_PATH}/src/ds/ui/touch/draw_touch_view.cpp
	${ROOT_PATH}/src/ds/ui/touch/touch_manager.cpp
	${ROOT_PATH}/src/ds/ui/touch/rotation_translator.cpp
	${ROOT_PATH}/src/ds/ui/touch/select_picking.cpp
	${ROOT_PATH}/src/ds/ui/touch/touch_translator.cpp
	${ROOT_PATH}/src/ds/ui/touch/touch_mode.cpp
	${ROOT_PATH}/src/ds/ui/tween/tweenline.cpp
	${ROOT_PATH}/src/ds/ui/tween/sprite_anim.cpp
	${ROOT_PATH}/src/ds/ui/service/glsl_image_service.cpp
	${ROOT_PATH}/src/ds/ui/service/pango_font_service.cpp
	${ROOT_PATH}/src/ds/ui/service/load_image_service.cpp
	${ROOT_PATH}/src/ds/ui/sprite/util/blend.cpp
	${ROOT_PATH}/src/ds/ui/sprite/util/clip_plane.cpp
	${ROOT_PATH}/src/ds/ui/sprite/sprite_engine.cpp
	${ROOT_PATH}/src/ds/ui/sprite/border.cpp
	${ROOT_PATH}/src/ds/ui/sprite/sprite.cpp
	${ROOT_PATH}/src/ds/ui/sprite/image.cpp
	${ROOT_PATH}/src/ds/ui/sprite/dirty_state.cpp
	${ROOT_PATH}/src/ds/ui/sprite/circle.cpp
	${ROOT_PATH}/src/ds/ui/sprite/shader/sprite_shader.cpp
	${ROOT_PATH}/src/ds/ui/sprite/gradient_sprite.cpp
	${ROOT_PATH}/src/ds/ui/sprite/mesh.cpp
	${ROOT_PATH}/src/ds/ui/sprite/nine_patch.cpp
	${ROOT_PATH}/src/ds/ui/sprite/text.cpp
	${ROOT_PATH}/src/ds/ui/sprite/image_with_thumbnail.cpp
	${ROOT_PATH}/src/ds/ui/sprite/circle_border.cpp
	${ROOT_PATH}/src/ds/ui/sprite/text_defs.cpp
	${ROOT_PATH}/src/ds/ui/ip/functions/ip_circle_mask.cpp
	${ROOT_PATH}/src/ds/ui/ip/ip_function.cpp
	${ROOT_PATH}/src/ds/ui/ip/ip_defs.cpp
	${ROOT_PATH}/src/ds/ui/ip/ip_function_list.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_client.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_glsl.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_drop_shadow.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_generator.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_source.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_owner.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_file.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_resource.cpp
	${ROOT_PATH}/src/ds/ui/image_source/image_arc.cpp
	${ROOT_PATH}/src/ds/ui/mesh_source/mesh_owner.cpp
	${ROOT_PATH}/src/ds/ui/mesh_source/mesh_source.cpp
	${ROOT_PATH}/src/ds/ui/mesh_source/mesh_cache_service.cpp
	${ROOT_PATH}/src/ds/ui/mesh_source/mesh_file_loader.cpp
	${ROOT_PATH}/src/ds/ui/mesh_source/mesh_sphere.cpp
	${ROOT_PATH}/src/ds/ui/mesh_source/mesh_file.cpp
	${ROOT_PATH}/src/ds/ui/layout/layout_sprite.cpp
	${ROOT_PATH}/src/ds/params/draw_params.cpp
	${ROOT_PATH}/src/ds/params/camera_params.cpp
	${ROOT_PATH}/src/ds/params/update_params.cpp
	${ROOT_PATH}/src/ds/debug/computer_info.cpp
	${ROOT_PATH}/src/ds/debug/debug_defines.cpp
	${ROOT_PATH}/src/ds/debug/logger.cpp
	${ROOT_PATH}/src/ds/math/math_func.cpp
	${ROOT_PATH}/src/ds/cfg/cfg_nine_patch.cpp
	${ROOT_PATH}/src/ds/cfg/settings.cpp
	${ROOT_PATH}/src/ds/cfg/cfg_text.cpp
	${ROOT_PATH}/src/ds/data/tuio_object.cpp
	${ROOT_PATH}/src/ds/data/resource_list.cpp
	${ROOT_PATH}/src/ds/data/key_value_store.cpp
	${ROOT_PATH}/src/ds/data/data_buffer.cpp
	${ROOT_PATH}/src/ds/data/read_write_buffer.cpp
	${ROOT_PATH}/src/ds/data/font_list.cpp
	${ROOT_PATH}/src/ds/data/color_list.cpp
	${ROOT_PATH}/src/ds/data/user_data.cpp
	${ROOT_PATH}/src/ds/data/resource.cpp

	${ROOT_PATH}/src/tuio/TuioClient.cpp
	${ROOT_PATH}/src/osc/OscBundle.cpp
	${ROOT_PATH}/src/osc/OscListener.cpp
	${ROOT_PATH}/src/osc/OscMessage.cpp
	${ROOT_PATH}/src/osc/OscSender.cpp
	${ROOT_PATH}/src/osc/ip/IpEndpointName.cpp
	${ROOT_PATH}/src/osc/ip/posix/NetworkingUtils.cpp
	${ROOT_PATH}/src/osc/ip/posix/UdpSocket.cpp
	${ROOT_PATH}/src/osc/osc/OscOutboundPacketStream.cpp
	${ROOT_PATH}/src/osc/osc/OscPrintReceivedElements.cpp
	${ROOT_PATH}/src/osc/osc/OscReceivedElements.cpp
	${ROOT_PATH}/src/osc/osc/OscTypes.cpp

	#from essentials
	${ROOT_PATH}/ds/debug/automator/actions/base_action.cpp
	${ROOT_PATH}/ds/debug/automator/actions/callback_action.cpp
	${ROOT_PATH}/ds/debug/automator/actions/drag_action.cpp
	${ROOT_PATH}/ds/debug/automator/actions/multifinger_tap_action.cpp
	${ROOT_PATH}/ds/debug/automator/actions/tap_action.cpp
	${ROOT_PATH}/ds/debug/automator/automator.cpp
	${ROOT_PATH}/ds/network/helper/delayed_node_watcher.cpp
	${ROOT_PATH}/ds/network/https_client.cpp
	${ROOT_PATH}/ds/network/smtp_request.cpp
	${ROOT_PATH}/ds/query/content/generic_content_model.cpp
	${ROOT_PATH}/ds/query/content/xml_content_loader.cpp
	${ROOT_PATH}/ds/touch/delayed_momentum.cpp
	${ROOT_PATH}/ds/touch/five_finger_cluster.cpp
	${ROOT_PATH}/ds/touch/touch_debug.cpp
	${ROOT_PATH}/ds/touch/view_dragger.cpp
	${ROOT_PATH}/ds/ui/button/image_button.cpp
	${ROOT_PATH}/ds/ui/button/layout_button.cpp
	${ROOT_PATH}/ds/ui/button/sprite_button.cpp
	${ROOT_PATH}/ds/ui/control/control_slider.cpp
	${ROOT_PATH}/ds/ui/drawing/drawing_canvas.cpp
	${ROOT_PATH}/ds/ui/interface_xml/interface_xml_importer.cpp
	${ROOT_PATH}/ds/ui/interface_xml/stylesheet_parser.cpp
	${ROOT_PATH}/ds/ui/layout/smart_layout.cpp
	${ROOT_PATH}/ds/ui/menu/component/cluster_view.cpp
	${ROOT_PATH}/ds/ui/menu/component/menu_item.cpp
	${ROOT_PATH}/ds/ui/menu/touch_menu.cpp
	${ROOT_PATH}/ds/ui/scroll/centered_scroll_area.cpp
	${ROOT_PATH}/ds/ui/scroll/infinity_scroll_list.cpp
	${ROOT_PATH}/ds/ui/scroll/scroll_area.cpp
	${ROOT_PATH}/ds/ui/scroll/scroll_bar.cpp
	${ROOT_PATH}/ds/ui/scroll/scroll_list.cpp
	${ROOT_PATH}/ds/ui/soft_keyboard/entry_field.cpp
	${ROOT_PATH}/ds/ui/soft_keyboard/soft_keyboard.cpp
	${ROOT_PATH}/ds/ui/soft_keyboard/soft_keyboard_builder.cpp
	${ROOT_PATH}/ds/ui/soft_keyboard/soft_keyboard_button.cpp
	${ROOT_PATH}/ds/ui/soft_keyboard/soft_keyboard_defs.cpp
	${ROOT_PATH}/ds/ui/sprite/donut_arc.cpp
	${ROOT_PATH}/ds/ui/sprite/png_sequence_sprite.cpp
	
)
]]
