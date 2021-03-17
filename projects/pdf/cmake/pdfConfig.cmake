if( NOT TARGET pdf )
	add_compile_definitions(-DUNICODE -D_UNICODE)
	get_filename_component( PDF_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../mupdf/src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )
	get_filename_component( MuPDF_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/../mupdf/lib" ABSOLUTE )
	get_filename_component( LIBRESOURCES_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/../lib64" ABSOLUTE )

	list( APPEND PDF_SRC_FILES
		${PDF_SRC_PATH}/private/pdf_service.cpp
		${PDF_SRC_PATH}/private/pdf_res.cpp
		${PDF_SRC_PATH}/ds/ui/sprite/pdf.cpp
		${PDF_SRC_PATH}/ds/ui/sprite/pdf_link.cpp
	)
	add_library( pdf ${PDF_SRC_FILES} )
	target_compile_features(pdf PUBLIC cxx_std_17)
	target_compile_definitions(pdf PUBLIC UNICODE _UNICODE)
	set_target_properties(pdf PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# Place compiled library in project's lib directory
	set_target_properties ( pdf PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( pdf PROPERTIES OUTPUT_NAME "pdf_d" )
	endif()

	target_include_directories( pdf PUBLIC "${PDF_SRC_PATH}" )
	target_include_directories( pdf SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )
	
	#MuPdf
	target_include_directories( pdf PRIVATE ${MuPDF_LIB_PATH})
	target_link_libraries( pdf PUBLIC
		${MuPDF_LIB_PATH}/MuPDF/lib64/${CMAKE_BUILD_TYPE}/libmupdf.lib
		${MuPDF_LIB_PATH}/MuPDF/lib64/${CMAKE_BUILD_TYPE}/thirdparty/libthirdparty.lib
		${LIBRESOURCES_LIB_PATH}/libresources.lib
	)
	# Mupdf
	#find_package( MuPDF REQUIRED )
	#target_include_directories( pdf SYSTEM BEFORE PRIVATE ${MuPDF_INCLUDE_DIRS} )
	#target_link_libraries( pdf PRIVATE ${MuPDF_LIBRARIES} )
	#target_link_libraries( pdf PRIVATE jpeg jbig -l:libjbig2dec.so.0 -l:libopenjpeg.so.5 )

	# Freetype
	#find_package( Freetype REQUIRED )
	#target_include_directories( pdf SYSTEM BEFORE PRIVATE ${Freetype_INCLUDE_DIRS} )
	#target_link_libraries( pdf PRIVATE ${Freetype_LIBRARIES} )

	# HarfBuzz
	#find_package( HarfBuzz REQUIRED )
	#target_include_directories( pdf SYSTEM BEFORE PRIVATE ${HARFBUZZ_INCLUDE_DIRS} )
	#target_link_libraries( pdf PRIVATE ${HARFBUZZ_LIBRARIES} )


	# pull in ds_cinder's exported configuration
	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( pdf PRIVATE ds-cinder-platform )


	#cinder
	include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
	set( LIBCINDER_LIB_DIRECTORY "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}v${MSVC_TOOLSET_VERSION}")
	target_link_libraries( video PRIVATE "${LIBCINDER_LIB_DIRECTORY}/cinder.lib" )
	target_include_directories( video PRIVATE "${CINDER_PATH}/include" )


	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( pdf PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( pdf )

endif()


