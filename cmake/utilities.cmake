function( _ds_build_message )
	# get the path of the CMakeLists file evaluating this macro relative to the project's root source directory
	file( RELATIVE_PATH _curr_file "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_LIST_FILE}" )

	# Build up all remaining arguments into the _msg var
	set( _msg )
	set( _msg "${_msg}${ARGV${0}}" )

	# deposit the concatenated message into 'msg' var for the parent function to use
	set( msg "(${_curr_file}) -- ${_msg}" PARENT_SCOPE )
endfunction()

# Only prints if DS_CINDER_VERBOSE is On
function( ds_log_v )
	if( DS_CINDER_VERBOSE )
		_ds_build_message( "${ARGV}" )
		message( "verbose ${msg}" )
	endif()
endfunction()

# Alawys prints
function( ds_log_i )
	_ds_build_message( "${ARGV}" )
	message( "info ${msg}" )
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

