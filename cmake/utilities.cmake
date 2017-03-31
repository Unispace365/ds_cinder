if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColorReset "${Esc}[m")
  set(ColorBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

function( _ds_build_message )
	set( options SECTION TRACE )
	set( oneValueArgs PREFIX )
	cmake_parse_arguments( BM_ARG  ""            "${oneValueArgs}" "" ${ARGN} )
	cmake_parse_arguments( LOG_ARG "${options}"  ""                "" ${ARGV0} )

	# get the path of the CMakeLists file evaluating this macro relative to the project's root source directory
	file( RELATIVE_PATH _curr_file "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_LIST_FILE}" )

	# Build up all remaining arguments into the _msg var
	set( _msg "${LOG_ARG_UNPARSED_ARGUMENTS}" )
	if ( LOG_ARG_TRACE )
		set( _msg "${_msg} -- (${_curr_file})" )
	endif()

	# deposit the concatenated message into 'msg' var for the parent function to use
	if ( LOG_ARG_SECTION )
		string (CONCAT _msg 
			"\n${BM_ARG_PREFIX}--------------------------------------------------\n"
			"${BM_ARG_PREFIX}  ${_msg}\n"
			"${BM_ARG_PREFIX}--------------------------------------------------\n"
		)
	else()
		set( _msg "${BM_ARG_PREFIX}${_msg}" )
	endif()
	set( msg "${_msg}" PARENT_SCOPE )
endfunction()

# Only prints if DS_CINDER_VERBOSE is On
function( ds_log_v )
	if( DS_CINDER_VERBOSE )
		_ds_build_message( "${ARGV}" PREFIX "[V] " )
		message( "${Blue}${msg}${ColorReset}" )
	endif()
endfunction()

# Always prints
function( ds_log_i )
	_ds_build_message( "${ARGV}" PREFIX "[I] "  )
	message( "${Green}${msg}${ColorReset}" )
endfunction()
function( ds_log_w )
	_ds_build_message( "${ARGV}" PREFIX "[W] " )
	message( WARNING "${Yellow}${msg}${ColorReset}" )
endfunction()
function( ds_log_e )
	_ds_build_message( "${ARGV}" PREFIX "[E] " )
	message( FATAL_ERROR "${Red}${msg}${ColorReset}" )
endfunction()


# Get all propreties that cmake supports
execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)
# Convert command output into a CMake list
STRING(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
STRING(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")

function(print_properties)
	message ("CMAKE_PROPERTY_LIST = ${CMAKE_PROPERTY_LIST}")
endfunction(print_properties)

function(print_target_properties tgt)
	if(NOT TARGET ${tgt})
	  message("There is no target named '${tgt}'")
	  return()
	endif()

	foreach (prop ${CMAKE_PROPERTY_LIST})
		string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" prop ${prop})
		# message ("Checking ${prop}")
		get_property(propval TARGET ${tgt} PROPERTY ${prop} SET)
		if (propval)
			get_target_property(propval ${tgt} ${prop})
			message ("${tgt} ${prop} = ${propval}")
		endif()
	endforeach(prop)
endfunction(print_target_properties)


# Make a symlink post-build
macro( MAKE_SYMLINK_POST_BUILD target src dest )
	add_custom_command( TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink ${dest} ${src}
		DEPENDS ${dest}
		COMMENT "mklink ${src} -> ${dest}" )
endmacro()

macro( MAKE_SYMLINKS_POST_BUILD target files_list files_dir links_dir )
	foreach( filename ${files_list} )
		set( source_name "${links_dir}/${filename}" )
		set( target_name "${files_dir}/${filename}" )
		MAKE_SYMLINK_POST_BUILD( ${target} ${source_name} ${target_name} )
	endforeach()
endmacro()
