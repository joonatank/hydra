project("tests" CXX)

cmake_minimum_required(VERSION 2.6)

include_directories( util )

subdirs(util
		spikes
		dotscene_loader
		eq_cluster
		udp
		)

add_executable( test_filestring
				test_filestring.cpp
				${eqOgre_SOURCE_DIR}/base/filesystem.hpp
				)

add_executable( test_settings 
				test_settings.cpp 
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

set( LIBS ${Boost_LIBRARIES} )

target_link_libraries( test_filestring ${LIBS} )
target_link_libraries( test_settings ${LIBS} ${OGRE_LIBRARIES} )
#target_link_libraries( test_dotscene ${LIBS} ${OGRE_LIBRARIES} )
target_link_libraries( test_exceptions ${LIBS} )
target_link_libraries( test_tracking ${LIBS} ${OGRE_LIBRARIES} vrpn )

add_test( filestring ${PROJECT_BINARY_DIR}/test_filestring )
add_test( settings ${PROJECT_BINARY_DIR}/test_settings )
add_test( dotscene ${PROJECT_BINARY_DIR}/test_dotscene )
