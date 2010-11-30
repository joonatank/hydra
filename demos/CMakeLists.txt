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

project( "demos" CXX )

cmake_minimum_required(VERSION 2.8)

message( "Thread libraries = ${CMAKE_THREAD_LIBS_INIT}" )
message( "VRPN libraries = ${VRPN_LIBRARY}" )

# Copy configs
copy_start_script( renderer
	ogre_project/ogre.prj
	"ogre" )

copy_start_script( renderer
	tracking_test_project/tracking.prj
	tracking )

add_executable( fake_tracking_server
	fake_tracking_server.cpp
	)

target_link_libraries( fake_tracking_server
	${VRPN_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT} )


add_executable( renderer
	listening_client.cpp
	)

target_link_libraries( renderer ${eqOgre_LIBRARY} )

add_executable( angles_to_quaternion
	angles_to_quaternion.cpp
	)

target_link_libraries( angles_to_quaternion ${OGRE_LIBRARIES} )


# install
install( TARGETS renderer fake_tracking_server DESTINATION ./ )
