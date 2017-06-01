cmake_minimum_required( VERSION 3.0 FATAL_ERROR )
#set( CMAKE_VERBOSE_MAKEFILE ON )

project( media_tester )

get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE )
get_filename_component( APP_PATH "${DS_CINDER_PATH}/example/${PROJECT_NAME}" ABSOLUTE )

include( "${DS_CINDER_PATH}/cmake/modules/dsCinderMakeApp.cmake" )

set( SRC_FILES
	${APP_PATH}/src/app/media_tester_app.cpp
)

ds_cinder_make_app(
	APP_PATH				${APP_PATH}
	SOURCES     			${SRC_FILES}
	DS_CINDER_PATH			${DS_CINDER_PATH}
	PROJECT_COMPONENTS     	essentials pdf video web
)
