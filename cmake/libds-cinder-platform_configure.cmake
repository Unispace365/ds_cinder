cmake_minimum_required( VERSION 3.0 FATAL_ERROR )

ds_log_v( "Building DsCinderPlatform for ${DS_CINDER_TARGET}" )

set( DS_CINDER_SRC_DIR 	"${DS_CINDER_PATH}/src" )
set( DS_CINDER_INC_DIR	"${DS_CINDER_PATH}/src" )

if( NOT DS_CINDER_MSW )
	add_definitions( -Wfatal-errors )
endif()

# Load cmake modules from our own cmake Modules path
list( APPEND CMAKE_MODULE_PATH ${DS_CINDER_CMAKE_DIR} ${CMAKE_CURRENT_LIST_DIR}/modules )

# Explanation of PUBLIC, PRIVATE, INTERFACE:
# From StackOverflow: http://stackoverflow.com/questions/31981602/what-interface-public-private-mean-in-cmake-target-compilation-setting
# These modes are useful if you have a target that is intended to be reused
# (usually a library).
#   * PRIVATE definitions only apply to the library target, but not to other
#	  targets that use this library. 
#   * INTERFACE definitions only apply to depending targets, but not to the
#	  library itself. 
#   * PUBLIC definitions apply both to the library target as wells as the
#	  depending targets.

# Explanation of SYSTEM vs USER:
# *_SYSTEM tells compiler that these are SYSTEM include directories, and will
# disable some warnings.  This translates into the -isystem flag passed to GCC,
# which will make GCC ignore any warnings generated inside these "system"
# headers
set(BOOST_DIR $ENV{BOOST_PATH} CACHE PATH "Boost location defaults to BOOST_PATH enviornment variable")
list( APPEND DS_CINDER_INCLUDE_SYSTEM_PUBLIC
	${DS_CINDER_INC_DIR}/tuio
	${DS_CINDER_INC_DIR}/osc
	
	${BOOST_DIR}
)

# *_INTERFACE includes get added to targets that depend on ds-cinder-platform.
list( APPEND DS_CINDER_INCLUDE_USER_INTERFACE
	${DS_CINDER_INC_DIR}
)

list( APPEND DS_CINDER_INCLUDE_SYSTEM_INTERFACE
	${DS_CINDER_INC_DIR}
	#${ROOT_PATH}/lib/poco/include
)
ds_log_v( "DS_CINDER_INCLUDE_USER_INTERFACE: ${DS_CINDER_INCLUDE_USER_INTERFACE}" )

# *_PRIVATE includes are used by ds-cinder-platform internally, user apps explicitly add these as needed.
list( APPEND DS_CINDER_INCLUDE_USER_PRIVATE
	${DS_CINDER_INC_DIR}
	${DS_CINDER_INC_DIR}/gtk
	${DS_CINDER_INC_DIR}/gtk/cairo
	${DS_CINDER_INC_DIR}/gtk/pango-1.0
	${DS_CINDER_INC_DIR}/gtk/glib-2.0
	${CINDER_PATH}/include
)

list( APPEND DS_CINDER_INCLUDE_SYSTEM_PRIVATE
	${DS_CINDER_INC_DIR}
)

# find cross-platform packages -> DS_CINDER_INCLUDE_SYSTEM_(PRIVATE|PUBLIC)
