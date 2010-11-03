project("tests" CXX)

cmake_minimum_required(VERSION 2.8)

find_package(Boost COMPONENTS unit_test_framework filesystem REQUIRED)

message( "Boost libraries = ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES};${Boost_FILESYSTEM_LIBRARIES}" )

include_directories( util ${Boost_INCLUDE_DIR} )

link_libraries( ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARIES}
	)

subdirs(util
		spikes
		dotscene_loader
		eq_cluster
		udp
		)

add_executable( test_settings 
				test_settings.cpp
				settings_fixtures.hpp
				${eqOgre_SOURCE_DIR}/settings.hpp
				${eqOgre_SOURCE_DIR}/settings.cpp
				${eqOgre_SOURCE_DIR}/base/args.cpp
				${eqOgre_SOURCE_DIR}/base/args.hpp
				)

#add_executable( test_dotscene test_dotscene.cpp )

add_executable( test_exceptions
				test_exceptions.cpp
				${eqOgre_SOURCE_DIR}/base/exceptions.hpp
				)

add_executable( test_tracking
				test_tracking.cpp
				${eqOgre_SOURCE_DIR}/tracker.hpp
				${eqOgre_SOURCE_DIR}/vrpn_tracker.hpp
				${eqOgre_SOURCE_DIR}/vrpn_tracker.cpp
				debug.hpp
				tracking_fixture.hpp
				)

#set( LIBS ${Boost_LIBRARIES} )

target_link_libraries( test_settings ${OGRE_LIBRARIES} )
#target_link_libraries( test_dotscene ${LIBS} ${OGRE_LIBRARIES} )
target_link_libraries( test_exceptions )
target_link_libraries( test_tracking ${OGRE_LIBRARIES} ${VRPN_LIBRARY} )

add_test( settings ${PROJECT_BINARY_DIR}/test_settings )
add_test( dotscene ${PROJECT_BINARY_DIR}/test_dotscene )
