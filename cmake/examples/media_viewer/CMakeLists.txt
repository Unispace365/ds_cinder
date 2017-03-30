cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
#set( CMAKE_VERBOSE_MAKEFILE ON )

project( media_viewer )

get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE )
get_filename_component( APP_PATH "${DS_CINDER_PATH}/example/${PROJECT_NAME}" ABSOLUTE )

include( "${DS_CINDER_PATH}/cmake/modules/dsCinderMakeExample.cmake" )

set( SRC_FILES
	${APP_PATH}/src/model/generated/media_model.cpp
	${APP_PATH}/src/query/query_handler.cpp
	${APP_PATH}/src/query/media_query.cpp
	${APP_PATH}/src/app/app_defs.cpp
	${APP_PATH}/src/app/globals.cpp
	${APP_PATH}/src/app/mediaviewer_app.cpp
	${APP_PATH}/src/ui/viewers/viewer_controller.cpp
	${APP_PATH}/src/ui/viewers/titled_media_viewer.cpp
)

ds_cinder_make_example(
	APP_PATH				${APP_PATH}
	SOURCES     			${SRC_FILES}
	DS_CINDER_PATH			${DS_CINDER_PATH}
	PROJECT_COMPONENTS     	essentials viewers
)
