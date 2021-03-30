cmake_minimum_required( VERSION 3.18min FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

set( DS_CINDER_PATH    		"${PROJECT_SOURCE_DIR}" )
set( DS_CINDER_CMAKE_DIR	"${PROJECT_SOURCE_DIR}/cmake" )


if( DS_CINDER_BUILD_EXAMPLES )
	include( ${DS_CINDER_CMAKE_DIR}/modules/findCMakeDirs.cmake )
	
	set( allExamples "" )
	findCMakeDirs( allExamples "${CMAKE_CURRENT_LIST_DIR}" "${DS_CINDER_SKIP_EXAMPLES}" )
	foreach( exampleDir ${allExamples} )
		ds_log_v( TRACE "adding example: ${exampleDir}" )
		add_subdirectory( ${exampleDir} "${exampleDir}/out")
	endforeach()
endif()