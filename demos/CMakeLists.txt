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

cmake_minimum_required(VERSION 2.6)

# Copy configs
copy_start_scripts( hydra
	ogre_project/ogre.prj
	"ogre" )

copy_start_scripts( hydra
	tracking_test_project/tracking.prj
	tracking )

copy_start_scripts( hydra
	large_test_project/large_test.prj
	large_test )

add_executable( fake_tracking_server
	fake_tracking_server.cpp
	)

target_link_libraries( fake_tracking_server
	${VRPN_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	${Ogre_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARIES}
	${Boost_SYSTEM_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARIES}
	)


add_executable( hydra hydra.cpp )

target_link_libraries( hydra ${HYDRA_LIBRARIES} )

add_executable( angles_to_quaternion
	angles_to_quaternion.cpp
	)

target_link_libraries( angles_to_quaternion ${Ogre_LIBRARY} )


# install
# install( TARGETS renderer fake_tracking_server DESTINATION ./ )
