#-------------------------------------------------------------------
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2010-11
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

project( "demos" CXX )

cmake_minimum_required(VERSION 2.6)

set(LIB_DIR "C:\\jotu\\software_development\\hydra_dependencies_libraries\\libs" CACHE PATH "Dependencies library dir")

# Copy configs
copy_start_scripts( hydra
	ogre_project/ogre.prj
	"ogre" )

copy_start_scripts( hydra
	"eye_tracking_test_project/eye_tracking_test.prj"
	"eye_tracking" )

copy_start_scripts( hydra
	light_test_project/light_test.prj
	"light_test" )

copy_start_scripts( hydra
	signal_test_project/signal_test.prj
	"signal_test" )

copy_start_scripts( hydra
	generated_physics_project/generated_physics.prj
	"generated_physics" )

copy_start_scripts( hydra
	robot_project/robot.prj
	"robot" )

copy_start_scripts( hydra
	perapora_project/perapora.prj
	"perapora" )

copy_start_scripts( hydra
	perapora_project/perapora_physics.prj
	"perapora_physics" )

copy_start_scripts( renderer
	tracking_test_project/tracking.prj
	tracking )

copy_start_scripts( hydra
	large_test_project/large_test.prj
	large_test )

add_executable( fake_tracking_server
	fake_tracking_server.cpp
	${HydraMain_SOURCE_DIR}/base/sleep.hpp
	${HydraMain_SOURCE_DIR}/base/timer.hpp
	${HydraMain_SOURCE_DIR}/base/timer.cpp
	${HydraMain_SOURCE_DIR}/math/conversion.hpp
	${HydraMain_SOURCE_DIR}/math/math.hpp
	${HydraMain_SOURCE_DIR}/math/math.cpp
	${HydraMain_SOURCE_DIR}/base/filesystem.cpp
	${HydraMain_SOURCE_DIR}/base/filesystem.hpp
	${HydraMain_SOURCE_DIR}/base/string_utils.cpp
	${HydraMain_SOURCE_DIR}/base/string_utils.hpp
	${HydraMain_SOURCE_DIR}/base/system_util.hpp
	${HydraMain_SOURCE_DIR}/base/system_util.cpp
	${HydraMain_SOURCE_DIR}/logger.hpp
	${HydraMain_SOURCE_DIR}/defines.hpp
	)

target_link_libraries(fake_tracking_server
	${Ogre_LIBRARY}
	${VRPN_LIBRARY}
	${Boost_FILESYSTEM_LIBRARIES}
	${Boost_PROGRAM_OPTIONS_LIBRARIES}
	)

add_executable(vrpn_tracker_recorder 
	vrpn_tracker_recorder.cpp
	${HydraMain_SOURCE_DIR}/base/sleep.hpp
	${HydraMain_SOURCE_DIR}/base/timer.hpp
	${HydraMain_SOURCE_DIR}/base/timer.cpp
	)

target_link_libraries(vrpn_tracker_recorder
	${VRPN_LIBRARY}
	${Boost_PROGRAM_OPTIONS_LIBRARIES}
	)

add_executable(vrpn_analog_server
	vrpn_analog_server.cpp
	${HydraMain_SOURCE_DIR}/base/sleep.hpp
	${HydraMain_SOURCE_DIR}/base/timer.hpp
	${HydraMain_SOURCE_DIR}/base/timer.cpp
	)

target_link_libraries(vrpn_analog_server ${VRPN_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARIES} )

add_executable(hydra hydra.cpp system_tools/message_box.hpp)

add_executable(angles_to_quaternion
	angles_to_quaternion.cpp
	${HydraMain_SOURCE_DIR}/math/math.cpp
	${HydraMain_SOURCE_DIR}/math/math.hpp)

target_link_libraries(angles_to_quaternion ${Ogre_LIBRARY})

# These should not need to link to anything else than boost
set( SRC
	${HydraMain_SOURCE_DIR}/base/timer.hpp
	${HydraMain_SOURCE_DIR}/base/timer.cpp
	${HydraMain_SOURCE_DIR}/base/report.hpp
	)
add_executable(timestamp_client timestamp_client.cpp ${SRC})
target_link_libraries( timestamp_client ${Boost_SYSTEM_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARIES} )

add_executable(timestamp_server timestamp_server.cpp ${SRC})
target_link_libraries( timestamp_server ${Boost_SYSTEM_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARIES} )

# install
# install( TARGETS renderer fake_tracking_server DESTINATION ./ )
