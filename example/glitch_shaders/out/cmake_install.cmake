# Install script for directory: C:/dev/ds_cinder_2019/example/glitch_shaders

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/dev/ds_cinder_2019/out/install/x64-Release")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE EXECUTABLE FILES "C:/dev/ds_cinder_2019/example/glitch_shaders/out/glitch_shaders.exe")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  set(DS_DEPENDENCY_DLL_PATHS "c:/dev/ds_cinder_2019/Cinder/lib/msw/x64;c:/dev/ds_cinder_2019/projects/mosquitto/lib64;c:/dev/ds_cinder_2019/projects/video/gstreamer-1.0/;c:/dev/ds_cinder_2019/projects/web/cef/lib64/runtime;C:/gstreamer/1.0/x86_64/bin")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  set(DS_OUT_DEPENDENCY_PATH "C:/dev/ds_cinder_2019/out/install/x64-Release")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  set(DS_CEF_PATH "c:/dev/ds_cinder_2019/projects/web/cef/lib64/runtime")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  set(DS_FONTCONFIG_PATH "c:/dev/ds_cinder_2019/lib/gtk/runtime64")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
      #find the dlls I need
    file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES C:/dev/ds_cinder_2019/example/glitch_shaders/out/glitch_shaders.exe
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
  
  

endif()

