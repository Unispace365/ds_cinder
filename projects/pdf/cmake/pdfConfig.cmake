if( NOT TARGET pdf )
	get_filename_component( PDF_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../mupdf/src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND PDF_SRC_FILES
		${PDF_SRC_PATH}/private/pdf_service.cpp
		${PDF_SRC_PATH}/private/pdf_res.cpp
		${PDF_SRC_PATH}/ds/ui/sprite/pdf.cpp
	)
	add_library( pdf ${PDF_SRC_FILES} )

	# Place compiled library in project's lib directory
	set_target_properties ( pdf PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( pdf PROPERTIES OUTPUT_NAME "pdf_d" )
	endif()

	target_include_directories( pdf PUBLIC "${PDF_SRC_PATH}" )
	target_include_directories( pdf SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )

	# Mupdf
	find_package( MuPDF REQUIRED )
	target_include_directories( pdf SYSTEM BEFORE PRIVATE ${MuPDF_INCLUDE_DIRS} )
	target_link_libraries( pdf PRIVATE ${MuPDF_LIBRARIES} )
	#target_link_libraries( pdf PRIVATE jpeg jbig -l:libjbig2dec.so.0 -l:libopenjpeg.so.5 )

	# Freetype
	find_package( Freetype REQUIRED )
	target_include_directories( pdf SYSTEM BEFORE PRIVATE ${Freetype_INCLUDE_DIRS} )
	target_link_libraries( pdf PRIVATE ${Freetype_LIBRARIES} )

	# pull in ds_cinder's exported configuration
	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( pdf PRIVATE ds-cinder-platform )


	# pull in cinder's exported configuration
	if( NOT TARGET cinder )
		include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		find_package( cinder REQUIRED PATHS
			"${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		)
	endif()
	target_link_libraries( pdf PUBLIC cinder )

	# Make building wai faster using Cotire
	list( APPEND CMAKE_MODULE_PATH ${DS_CINDER_PATH}/cmake/modules  )
	include( cotire )
	# TODO
	#set_target_properties( pdf PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( pdf )

endif()


