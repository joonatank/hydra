#-------------------------------------------------------------------
# Joonatan Kuosa
# 2010-11
#
# This file is part of the CMake build system for eqOgre
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

project( "spikes" CXX )

cmake_minimum_required(VERSION 2.8)


add_executable( spike_udp_renderer
	udp_renderer.cpp
	client_fixtures.hpp
	)

target_link_libraries( spike_udp_renderer ${eqOgre_LIBRARY} )

find_package( Bullet REQUIRED )

include_directories(
	${BULLET_INCLUDE_DIRS}
	${BULLET_INCLUDE_DIRS}/bullet
	)

add_executable( spike_bullet_test
	bullet_test.cpp
	)

target_link_libraries( spike_bullet_test ${BULLET_LIBRARIES} )

add_executable( spike_bullet
	bullet_spike.cpp
	physics_events.cpp
	)

target_link_libraries( spike_bullet ${BULLET_LIBRARIES} ${eqOgre_LIBRARY} )

copy_start_script( spike_bullet ogre_project/ogre.prj physics )
