cmake_minimum_required( VERSION 3.18 FATAL_ERROR )

# Project start
project( example_client_and_server )

file(TO_CMAKE_PATH $ENV{DS_PLATFORM_093} ENV_DS_PLATFORM_093)
file(TO_CMAKE_PATH $ENV{GSTREAMER_1_0_ROOT_x86_64} ENV_GSTREAMER_1_0_ROOT_x86_64)
set(DS_CINDER_PACKAGE_PATH "${ENV_DS_PLATFORM_093}/out/build/${CMAKE_BUILD_TYPE}/lib/ds_cinder/configs")

find_package( ds-cinder-platform PATHS "${DS_CINDER_PACKAGE_PATH}" )
find_package( video PATHS "${DS_CINDER_PACKAGE_PATH}")
find_package( web PATHS "${DS_CINDER_PACKAGE_PATH}")
find_package( pdf PATHS "${DS_CINDER_PACKAGE_PATH}" )
find_package( viewers PATHS "${DS_CINDER_PACKAGE_PATH}")

add_executable( example_client_and_server WIN32
    src/custom_sprite.cpp
    src/custom_sprite.h
    src/example_app.cpp
    src/example_app.h

)

set_target_properties(example_client_and_server PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries( example_client_and_server PUBLIC ds-cinder-platform video web pdf viewers)

target_include_directories( example_client_and_server PRIVATE
    ds-cinder-platform
    viewers
    video
    "src"
   
)

list(APPEND DS_DEPENDENCY_DLL_PATHS
    ${ENV_DS_PLATFORM_093}/Cinder/lib/msw/x64
    ${ENV_DS_PLATFORM_093}/projects/mosquitto/lib64
    ${ENV_DS_PLATFORM_093}/projects/video/gstreamer-1.0/
    ${ENV_DS_PLATFORM_093}/projects/web/cef/lib64/runtime 
    ${ENV_GSTREAMER_1_0_ROOT_x86_64}/bin
)

add_custom_command(
    TARGET example_client_and_server
    POST_BUILD
    COMMAND cmake ARGS -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
    WORKING_DIRECTORY 
)

install(TARGETS example_client_and_server RUNTIME DESTINATION ".")
install(CODE "set(DS_DEPENDENCY_DLL_PATHS \"${DS_DEPENDENCY_DLL_PATHS}\")")
install(CODE "set(DS_OUT_DEPENDENCY_PATH \"${CMAKE_INSTALL_PREFIX}\")")
install(CODE "set(DS_CEF_PATH \"${ENV_DS_PLATFORM_093}/projects/web/cef/lib64/runtime\")")
install(CODE "set(DS_FONTCONFIG_PATH \"${ENV_DS_PLATFORM_093}/lib/gtk/runtime64\")")
install(CODE [[
    #find the dlls I need
    file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES $<TARGET_FILE:example_client_and_server>
        RESOLVED_DEPENDENCIES_VAR _r_deps
        UNRESOLVED_DEPENDENCIES_VAR _u_deps
        POST_EXCLUDE_REGEXES ^[Cc]:[/\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\].*
        DIRECTORIES ${DS_DEPENDENCY_DLL_PATHS}
    )

    #copy the dlls
    foreach(_file ${_r_deps})
        file(INSTALL
            DESTINATION "${DS_OUT_DEPENDENCY_PATH}"
            TYPE SHARED_LIBRARY
            FILES "${_file}"
        )
    endforeach()

    list(LENGTH _u_deps _u_length)
    if("${_u_length}" GREATER 0)
        message(WARNING "Unresolved dependencies detected!")
    endif()

    #get needed cef files 
    file(GLOB_RECURSE CEF_FILES 
        LIST_DIRECTORIES true
        RELATIVE ${DS_CEF_PATH}
        "${DS_CEF_PATH}/*"
    )

    #copy needed cef files
    foreach(_cef_file ${CEF_FILES})
        get_filename_component(_cef_out_path "${DS_OUT_DEPENDENCY_PATH}/${_cef_file}" DIRECTORY)
        file(COPY "${DS_CEF_PATH}/${_cef_file}"
            DESTINATION "${_cef_out_path}"
        )
    endforeach()

    #get needed fontconfig files
    
    #file(GLOB_RECURSE FONTCONFIG_FILES 
    #    LIST_DIRECTORIES true
    #    RELATIVE ${DS_FONTCONFIG_PATH}
    #    "${DS_FONTCONFIG_PATH}/etc"
    #)
    #message ("Files: ${FONTCONFIG_FILES}")
    
    #copy needed fontconfig files
    file(COPY "${DS_FONTCONFIG_PATH}/etc"
        DESTINATION "${DS_OUT_DEPENDENCY_PATH}"
    )
  
  
]])

















