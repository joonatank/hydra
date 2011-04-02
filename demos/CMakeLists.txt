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

target_link_libraries( fake_tracking_server ${HYDRA_LIBRARIES} )


add_executable( hydra hydra.cpp )

target_link_libraries( hydra ${HYDRA_LIBRARIES} )

add_executable( angles_to_quaternion
	angles_to_quaternion.cpp
	)

target_link_libraries( angles_to_quaternion ${HYDRA_LIBRARIES} )

# These should not need to link to anything else than boost
set( SRC
	${vl_SOURCE_DIR}/vl/base/timer.hpp
	${vl_SOURCE_DIR}/vl/base/timer.cpp
	${vl_SOURCE_DIR}/vl/base/report.hpp
	)
add_executable(timestamp_client timestamp_client.cpp ${SRC})
target_link_libraries( timestamp_client ${Boost_SYSTEM_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARIES} )

add_executable(timestamp_server timestamp_server.cpp ${SRC})
target_link_libraries( timestamp_server ${Boost_SYSTEM_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARIES} )

# install
# install( TARGETS renderer fake_tracking_server DESTINATION ./ )
