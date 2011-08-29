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
	tracker.cpp
	vrpn_tracker.cpp
	tracker_serializer.cpp
	settings.cpp
	ogre_xml_helpers.cpp
	resource.cpp
	recording.cpp
	game_manager.cpp
	player.cpp
	dotscene_loader.cpp
	action.cpp
	actions_misc.cpp
	actions_transform.cpp
	scene_manager.cpp
	scene_node.cpp
	trigger.cpp
	event_manager.cpp
	resource_manager.cpp
	distrib_settings.cpp
	program_options.cpp
	stats.cpp
	logger.cpp
	window.cpp
	config.cpp
	renderer.cpp
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
	constraints.cpp
	constraints_handlers.cpp
	constraint_solver.cpp
	animation.cpp
	ray_object.cpp
	ray_cast.cpp
	)

set(MATH_HEADERS
	math/conversion.hpp
	math/math.hpp
	math/transform.hpp
	math/types.hpp
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
		input/joystick_event.hpp
		input/serial_joystick.hpp
		)
	set(INPUT_SRC
		input/serial_joystick.cpp
		)
	source_group(HydraMain\\input FILES ${INPUT_HEADERS} ${INPUT_SRC})
else()
	set(INPUT_HEADERS "")
	set(INPUT_SRC "")
endif()

set(BASE_HEADERS 
	base/thread.hpp
	base/system_util.hpp
	base/envsettings.hpp
	base/projsettings.hpp
	base/exceptions.hpp
	base/filesystem.hpp
	base/print.hpp
	base/rapidxml.hpp
	base/rapidxml_print.hpp
	base/sleep.hpp
	base/string_utils.hpp
	base/timer.hpp
	base/report.hpp)
set(BASE_SRC
	base/system_util.cpp
	base/filesystem.cpp
	base/envsettings.cpp
	base/projsettings.cpp
	base/string_utils.cpp
	base/timer.cpp
	)
if(WIN32)
	list(APPEND BASE_SRC base/serial.cpp)
	list(APPEND BASE_HEADERS base/serial.hpp)
endif()
source_group(HydraMain\\base FILES ${BASE_HEADERS} ${BASE_SRC})

set(PHYSICS_HEADERS
	physics/motion_state.hpp
	physics/rigid_body.hpp
	physics/shapes.hpp
	physics/physics_constraints.hpp
	physics/physics_events.hpp
	physics/physics_world.hpp
	physics/tube.hpp
	physics/mesh_bullet.hpp
	)
set(PHYSICS_SRC
	physics/physics_events.cpp
	physics/physics_world.cpp
	physics/physics_constraints.cpp
	physics/shapes.cpp
	physics/rigid_body.cpp
	physics/tube.cpp
	physics/mesh_bullet.cpp
	)
if(HYDRA_PHYSICS STREQUAL "Bullet")
	list(APPEND PHYSICS_HEADERS
		physics/physics_world_bullet.hpp
		physics/rigid_body_bullet.hpp
		physics/shapes_bullet.hpp
		physics/physics_constraints_bullet.hpp
		)
	list(APPEND PHYSICS_SRC
		physics/physics_world_bullet.cpp
		)
elseif(HYDRA_PHYSICS STREQUAL "Newton")
	list(APPEND PHYSICS_HEADERS
		physics/physics_world_newton.hpp
		physics/rigid_body_newton.hpp
		physics/shapes_newton.hpp
		physics/physics_constraints_newton.hpp
		)
	list(APPEND PHYSICS_SRC
		physics/physics_world_newton.cpp
		)
endif()

source_group(HydraMain\\physics FILES ${PHYSICS_HEADERS} ${PHYSICS_SRC})

set(PYTHON_HEADERS
	python/python.hpp
	python/python_module.hpp
	)

set(PYTHON_SRC
	python/python.cpp
	python/python_module.cpp
	python/python_events.cpp
	python/python_physics.cpp
	)
source_group(HydraMain\\python FILES ${PYTHON_HEADERS} ${PYTHON_SRC})

set( HEADERS
	system_tools/message_box.hpp
	tracker.hpp
	entity.hpp
	light.hpp
	movable_object.hpp
	movable_text.hpp
	serializer.hpp
	mesh_serializer.hpp
	mesh_serializer_impl.hpp
	mesh.hpp
	mesh_ogre.hpp
	mesh_manager.hpp
	camera.hpp
	tracker_serializer.hpp
	vrpn_tracker.hpp
	settings.hpp
	ogre_xml_helpers.hpp
	game_manager.hpp
	player.hpp
	stats.hpp
	typedefs.hpp
	resource.hpp
	recording.hpp
	resource_manager.hpp
	dotscene_loader.hpp
	action.hpp
	actions_misc.hpp
	actions_transform.hpp
	scene_manager.hpp
	scene_node.hpp
	trigger.hpp
	event_manager.hpp
	keycode.hpp
	session.hpp
	logger.hpp
	distrib_settings.hpp
	program_options.hpp
	window.hpp
	config.hpp
	renderer.hpp
	renderer_interface.hpp
	application.hpp
	ogre_root.hpp
	distributed.hpp
	constraints.hpp
	constraints_handlers.hpp
	constraint_solver.hpp
	animation.hpp
	ray_object.hpp
	ray_cast.hpp
	)

source_group(HydraMain FILES ${HEADERS} ${SRC})

set(GUI_HEADERS
	gui/gui.hpp
	gui/gui_window.hpp
	gui/gui_actions.hpp
	)

set(GUI_SRC
	gui/gui.cpp
	gui/gui_window.cpp
	)
source_group(HydraMain\\GUI FILES ${GUI_HEADERS} ${GUI_SRC})

set(CLUSTER_HEADERS
	cluster/server.hpp
	cluster/client.hpp
	cluster/message.hpp
	cluster/states.hpp
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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	message("CXX flags ${CMAKE_CXX_FLAGS}")
endif()

add_library(HydraMain
	${SRC}
	${HEADERS}
	${BASE_HEADERS}
	${BASE_SRC}
	${INPUT_HEADERS}
	${INPUT_SRC}
	${PHYSICS_HEADERS}
	${PHYSICS_SRC}
	${PYTHON_HEADERS}
	${PYTHON_SRC}
	${CLUSTER_HEADERS}
	${CLUSTER_SRC}
	${GUI_HEADERS}
	${GUI_SRC}
	${MATH_HEADERS}
	${MATH_SRC}
	)

target_link_libraries(HydraMain ${HYDRA_DEP_LIBS})

set_target_properties(HydraMain PROPERTIES DEBUG_POSTFIX _d)

#install
# TODO this is here for test only the filestructure is not preserved by this
#install( FILES ${HEADERS} DESTINATION include )
