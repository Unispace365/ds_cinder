if( NOT TARGET physics )
	get_filename_component( PHYSICS_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../box2d/src" ABSOLUTE )
	get_filename_component( DS_CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE )

	list( APPEND PHYSICS_SRC_FILES
		${PHYSICS_SRC_PATH}/ds/physics/body_builder.cpp
		${PHYSICS_SRC_PATH}/ds/physics/collision.cpp
		${PHYSICS_SRC_PATH}/ds/physics/sprite_body.cpp
		${PHYSICS_SRC_PATH}/ds/physics/sprite_world.cpp
		${PHYSICS_SRC_PATH}/ds/physics/contact_key.cpp
		${PHYSICS_SRC_PATH}/ds/physics/contact_listener.cpp
		${PHYSICS_SRC_PATH}/ds/physics/debug_draw.cpp
		${PHYSICS_SRC_PATH}/ds/physics/service.cpp
		${PHYSICS_SRC_PATH}/ds/physics/touch.cpp
		${PHYSICS_SRC_PATH}/ds/physics/world.cpp
	)

	# This project actually includes all the lib dependency's sources (Box2D)..
	get_filename_component( BOX2D_SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/../box2d/lib" ABSOLUTE )
	list( APPEND PHYSICS_SRC_FILES
		${BOX2D_SRC_PATH}/Box2D/Rope/b2Rope.cpp
		${BOX2D_SRC_PATH}/Box2D/Common/b2Settings.cpp
		${BOX2D_SRC_PATH}/Box2D/Common/b2BlockAllocator.cpp
		${BOX2D_SRC_PATH}/Box2D/Common/b2Draw.cpp
		${BOX2D_SRC_PATH}/Box2D/Common/b2Math.cpp
		${BOX2D_SRC_PATH}/Box2D/Common/b2StackAllocator.cpp
		${BOX2D_SRC_PATH}/Box2D/Common/b2Timer.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2CircleContact.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2PolygonContact.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2ContactSolver.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Contacts/b2Contact.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/b2WorldCallbacks.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/b2Fixture.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2WeldJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2DistanceJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2FrictionJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2GearJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2MotorJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2RopeJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2Joint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2WheelJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2PulleyJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/Joints/b2MouseJoint.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/b2World.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/b2Body.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/b2ContactManager.cpp
		${BOX2D_SRC_PATH}/Box2D/Dynamics/b2Island.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/b2CollidePolygon.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/b2CollideEdge.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/b2Distance.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/b2DynamicTree.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/Shapes/b2PolygonShape.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/Shapes/b2CircleShape.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/Shapes/b2ChainShape.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/Shapes/b2EdgeShape.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/b2BroadPhase.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/b2TimeOfImpact.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/b2Collision.cpp
		${BOX2D_SRC_PATH}/Box2D/Collision/b2CollideCircle.cpp
	)

	add_library( physics ${PHYSICS_SRC_FILES} )
	target_compile_features(physics PUBLIC cxx_std_17)
	target_compile_definitions(physics PUBLIC UNICODE _UNICODE)
	set_target_properties(physics PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# Place compiled library in project's lib directory
	set_target_properties ( physics PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../lib )
	# Add "_d" to library name for debug builds
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		set_target_properties ( physics PROPERTIES OUTPUT_NAME "physics_d" )
	endif()

	target_include_directories( physics PUBLIC "${PHYSICS_SRC_PATH}" )
	target_include_directories( physics SYSTEM BEFORE PUBLIC "${DS_CINDER_PATH}/src" )

	target_include_directories( physics SYSTEM PRIVATE "${BOX2D_SRC_PATH}" )

	# pull in ds_cinder's exported configuration
	if( NOT TARGET ds-cinder-platform )
		include( "${DS_CINDER_PATH}/cmake/configure.cmake" )
		find_package( ds-cinder-platform REQUIRED PATHS
			"${DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}"
			"$ENV{DS_CINDER_PATH}/${DS_CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( physics PRIVATE ds-cinder-platform )


	# pull in cinder's exported configuration
	set( LIBCINDER_LIB_DIRECTORY "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}v${MSVC_TOOLSET_VERSION}")
	target_link_libraries( mosquitto PRIVATE "${LIBCINDER_LIB_DIRECTORY}/cinder.lib" )
	target_include_directories( mosquitto PRIVATE "${CINDER_PATH}/include" )

	# Make building wai faster using Cotire
	include( cotire )
	# TODO
	#set_target_properties( physics PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${ROOT_PATH}/src/stdafx.h" )
	#cotire( physics )

endif()


