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

project("tests" CXX)

cmake_minimum_required(VERSION 2.6)

enable_testing()

find_package(Boost COMPONENTS unit_test_framework filesystem REQUIRED)

set( FS_LIB ${Boost_FILESYSTEM_LIBRARIES} )
set( TEST_LIB ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES} )

######### Find Turtle ########
# We should expose these parameters for the user even in Linux
# We should use find_file and file_lib functions to find the files first though

find_path( TURTLE_INCLUDE_DIR
	NAMES turtle/mock.hpp
	PATHS $ENV{TURTLE_HOME} $ENV{TURTLE_ROOT} $ENV{TURTLE_ROOT} $ENV{TURTLE_HOME}
	)

# TODO add link variable so not all programs will link to these
link_libraries( ${FS_LIB} ${TEST_LIB} )

include_directories( SYSTEM
	${TURTLE_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	)

add_executable( test_timer test_timer.cpp
	${HydraMain_SOURCE_DIR}/base/timer.hpp
	${HydraMain_SOURCE_DIR}/base/timer.cpp
	)

#subdirs( spikes )

# NOTE For now removed as we move to using EnvSettings and ProjectSettings
#add_executable( test_settings
#				test_settings.cpp
#				settings_fixtures.hpp
#				${eqOgre_SOURCE_DIR}/settings.hpp
#				${eqOgre_SOURCE_DIR}/settings.cpp
#				${eqOgre_SOURCE_DIR}/base/args.cpp
#				${eqOgre_SOURCE_DIR}/base/args.hpp
#				)

#target_link_libraries( test_settings ${OGRE_LIBRARIES} ${TEST_LIB} ${FS_LIB} )
#add_test( settings ${PROJECT_BINARY_DIR}/test_settings )

# FIXME dotscene testing
#add_executable( test_dotscene test_dotscene.cpp )
#target_link_libraries( test_dotscene ${LIBS} ${OGRE_LIBRARIES} )
#add_test( dotscene ${PROJECT_BINARY_DIR}/test_dotscene )

# Test exceptions
add_executable( test_exceptions
				test_exceptions.cpp
				${HydraMain_SOURCE_DIR}/base/exceptions.hpp
				)

# Is this needed
target_link_libraries( test_exceptions ${TEST_LIB} )

# Test euler angles
add_executable( test_euler_angles
				test_euler_angles.cpp
				${HydraMain_SOURCE_DIR}/math/math.hpp
				)

target_link_libraries( test_euler_angles ${HYDRA_LIBRARIES} ${TEST_LIB} )

# Test tracking
#add_executable( test_tracking
#				test_tracking.cpp
#				debug.hpp
#				tracking_fixture.hpp
#				)

# target_link_libraries( test_tracking
# 	eqOgre
# 	${TEST_LIB}
# 	)



# Test env settings
add_executable( test_env_settings
    test_env_settings.cpp
    ${HydraMain_SOURCE_DIR}/base/envsettings.cpp
    ${HydraMain_SOURCE_DIR}/base/envsettings.hpp
    )

target_link_libraries( test_env_settings ${TEST_LIB} ${FS_LIB} )

add_test( env_settings ${PROJECT_BINARY_DIR}/test_env_settings )


# Test project settings
# FIXME these use old interface
##add_executable( test_project_settings
##        test_project_settings.cpp
##        ${vl_SOURCE_DIR}/base/projsettings.cpp
##        ${vl_SOURCE_DIR}/base/projsettings.h
##        )

##target_link_libraries( test_project_settings ${Boost_LIBRARIES} )

##add_test( project_settings ${PROJECT_BINARY_DIR}/test_project_settings )



