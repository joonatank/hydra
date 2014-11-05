#-------------------------------------------------------------------
# Joonatan Kuosa
# 2010-11
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

project( "HydraMain" CXX )

cmake_minimum_required(VERSION 2.6)

set(SRC
	object_interface.cpp
	settings.cpp
	resource.cpp
	game_manager.cpp
	player.cpp
	dotscene_loader.cpp
	scene_manager.cpp
	scene_node.cpp
	trigger.cpp
	event_manager.cpp
	resource_manager.cpp
	distrib_settings.cpp
	program_options.cpp
	logger.cpp
	window.cpp
	channel.cpp
	stereo_camera.cpp
	master.cpp
	slave.cpp
	renderer.cpp
	pipe.cpp
	application.cpp
	ogre_root.cpp
	entity.cpp
	light.cpp
	camera.cpp
	movable_object.cpp
	movable_text.cpp
	serializer.cpp
	mesh_serializer.cpp
	mesh_serializer_impl.cpp
	mesh_manager.cpp
	mesh.cpp
	mesh_ogre.cpp
	material.cpp
	material_manager.cpp
	ray_object.cpp
	ray_cast_ogre.cpp
	sky_skyx.cpp
	ogre_axes.cpp
	remote_launcher_helper.cpp
	game_object.cpp
	hsf_loader.cpp
	hsf_writer.cpp
	profiler_report.cpp
	eye_tracker.cpp
	cad_importer.cpp
	)

set(MATH_HEADERS
	math/conversion.hpp
	math/math.hpp
	math/transform.hpp
	math/types.hpp
	math/colour.hpp
	math/frustum.hpp
	math/frustum.cpp
	)
set(MATH_SRC
	math/math.cpp
	math/transform.cpp
	)
source_group(HydraMain\\math FILES ${MATH_HEADERS} ${MATH_SRC})

# @todo no Linux version for now
if(WIN32)
	set(INPUT_HEADERS
		input/input.hpp
		input/ois_converters.hpp
		input/pcan.hpp
		input/mouse_event.hpp
		input/joystick_event.hpp
		input/tracker.hpp
		input/tracker_serializer.hpp
		input/vrpn_tracker.hpp
		input/vrpn_analog_client.hpp
		input/keycode.hpp
		)
	set(INPUT_SRC
		input/pcan.cpp
		input/tracker.cpp
		input/vrpn_tracker.cpp
		input/vrpn_analog_client.cpp
		input/tracker_serializer.cpp
		)
	source_group(HydraMain\\input FILES ${INPUT_HEADERS} ${INPUT_SRC})
else()
	set(INPUT_HEADERS "")
	set(INPUT_SRC "")
endif()

set(BASE_HEADERS
	base/system_util.hpp
	base/envsettings.hpp
	base/projsettings.hpp
	base/config.hpp
	base/rect.hpp
	base/exceptions.hpp
	base/filesystem.hpp
	base/print.hpp
	base/rapidxml.hpp
	base/rapidxml_print.hpp
	base/sleep.hpp
	base/string_utils.hpp
	base/time.hpp
	base/chrono.hpp
	base/report.hpp
	base/wall.hpp
	base/state_machines.hpp
	base/xml_helpers.hpp
	)
set(BASE_SRC
	base/system_util.cpp
	base/filesystem.cpp
	base/envsettings.cpp
	base/projsettings.cpp
	base/config.cpp
	base/string_utils.cpp
	base/time.cpp
	base/chrono.cpp
	base/xml_helpers.cpp
	)
if(WIN32)
	list(APPEND BASE_SRC base/serial.cpp)
	list(APPEND BASE_HEADERS base/serial.hpp)
endif()
source_group(HydraMain\\base FILES ${BASE_HEADERS} ${BASE_SRC})


set(COLLADA_HEADERS
	collada/dae_common.hpp
	collada/dae_importer.hpp
	collada/dae_exporter.hpp
	collada/dae_mesh_importer.hpp)
set(COLLADA_SRC
	collada/dae_importer.cpp
	collada/dae_exporter.cpp
	collada/dae_mesh_importer.cpp)
source_group(HydraMain\\collada FILES ${COLLADA_HEADERS} ${COLLADA_SRC})

set(PHYSICS_HEADERS
	physics/motion_state.hpp
	physics/motion_state_bullet.hpp
	physics/rigid_body.hpp
	physics/shapes.hpp
	physics/physics_constraints.hpp
	physics/physics_world.hpp
	physics/tube.hpp
	physics/physics_world_bullet.hpp
	physics/rigid_body_bullet.hpp
	physics/shapes_bullet.hpp
	physics/physics_constraints_bullet.hpp
	physics/mesh_bullet.hpp
	)
set(PHYSICS_SRC
	physics/motion_state.cpp
	physics/physics_world.cpp
	physics/physics_constraints.cpp
	physics/shapes.cpp
	physics/rigid_body.cpp
	physics/tube.cpp
	physics/physics_world_bullet.cpp
	physics/physics_constraints_bullet.cpp
	physics/mesh_bullet.cpp
	)

source_group(HydraMain\\physics FILES ${PHYSICS_HEADERS} ${PHYSICS_SRC})

set(ANIMATION_HEADERS
	animation/constraints.hpp
	animation/animation.hpp
	animation/kinematic_body.hpp
	animation/kinematic_world.hpp
	)
set(ANIMATION_SRC
	animation/constraints.cpp
	animation/animation.cpp
	animation/kinematic_body.cpp
	animation/kinematic_world.cpp
	)
source_group(HydraMain\\animation FILES ${ANIMATION_HEADERS} ${ANIMATION_SRC})


set(PYTHON_HEADERS
	python/python_context.hpp
	python/python_context_impl.hpp
	python/python_module.hpp
	)

set(PYTHON_SRC
	python/python_context_impl.cpp
	python/python_module.cpp
	python/python_events.cpp
	python/python_physics.cpp
	)
source_group(HydraMain\\python FILES ${PYTHON_HEADERS} ${PYTHON_SRC})

set( HEADERS
	defines.hpp
	object_interface.hpp
	entity.hpp
	light.hpp
	camera.hpp
	movable_object.hpp
	movable_text.hpp
	serializer.hpp
	mesh_serializer.hpp
	mesh_serializer_impl.hpp
	mesh.hpp
	mesh_ogre.hpp
	mesh_manager.hpp
	material.hpp
	material_manager.hpp
	settings.hpp
	game_manager.hpp
	player.hpp
	typedefs.hpp
	resource.hpp
	resource_manager.hpp
	dotscene_loader.hpp
	scene_manager.hpp
	scene_node.hpp
	trigger.hpp
	event_manager.hpp
	logger.hpp
	distrib_settings.hpp
	program_options.hpp
	window.hpp
	window_interface.hpp
	channel.hpp
	stereo_camera.hpp
	master.hpp
	slave.hpp
	renderer.hpp
	pipe.hpp
	application.hpp
	ogre_root.hpp
	ray_object.hpp
	ray_cast_ogre.hpp
	sky_interface.hpp
	sky_skyx.hpp
	game_object.hpp
	hsf_loader.hpp
	hsf_writer.hpp
	ogre_axes.hpp
	remote_launcher_helper.hpp
	profiler_report.hpp
	flags.hpp
	eye_tracker.hpp
	cad_importer.hpp
	)

source_group(HydraMain FILES ${HEADERS} ${SRC})

set(GUI_HEADERS
	gui/gui.hpp
	gui/gui_window.hpp
	gui/console.hpp
	gui/performance_overlay.hpp
	gui/Gorilla.h
	)

set(GUI_SRC
	gui/gui.cpp
	gui/gui_window.cpp
	gui/console.cpp
	gui/performance_overlay.cpp
	gui/Gorilla.cpp
	)
source_group(HydraMain\\GUI FILES ${GUI_HEADERS} ${GUI_SRC})

set(CLUSTER_HEADERS
	cluster/server.hpp
	cluster/client.hpp
	cluster/message.hpp
	cluster/states.hpp
	cluster/session.hpp
	cluster/object_types.hpp
	cluster/distributed.hpp
	)

set(CLUSTER_SRC
	cluster/server.cpp
	cluster/client.cpp
	cluster/message.cpp
	)
source_group(HydraMain\\cluster FILES ${CLUSTER_HEADERS} ${CLUSTER_SRC})

# Set parallel build
# TODO make configurable, maybe move to solution level
if(WIN32)
	message(STATUS "Using multiple cores for building")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

add_definitions(-DHYDRA_SHARED_LIB)

add_library(HydraMain SHARED
	${SRC}
	${HEADERS}
	${BASE_HEADERS}
	${BASE_SRC}
	${COLLADA_HEADERS}
	${COLLADA_SRC}
	${INPUT_HEADERS}
	${INPUT_SRC}
	${PHYSICS_HEADERS}
	${PHYSICS_SRC}
	${ANIMATION_HEADERS}
	${ANIMATION_SRC}
	${PYTHON_HEADERS}
	${PYTHON_SRC}
	${CLUSTER_HEADERS}
	${CLUSTER_SRC}
	${GUI_HEADERS}
	${GUI_SRC}
	${MATH_HEADERS}
	${MATH_SRC}
	)

# Generate Revision information from Mercurial
# For this to work Mercurial needs to be on the windows Path
add_custom_command(TARGET  HydraMain PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/revision.cmake)

include_directories(${HydraMain_BINARY_DIR})

target_link_libraries(HydraMain ${HYDRA_DEP_LIBS})

set_target_properties(HydraMain PROPERTIES DEBUG_POSTFIX _d)

# Only install the dynamic library, no headers because install is for runtime
# not for SDK.
# TODO this also copies libs which we don't need
install( TARGETS HydraMain DESTINATION bin CONFIGURATIONS Release )

