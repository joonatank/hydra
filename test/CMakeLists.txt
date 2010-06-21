project("tests" CXX)

cmake_minimum_required(VERSION 2.6)

include_directories( util )

subdirs(util
		math
		eq_graph
		spikes
#		eq_ogre
		synchronization
		dotscene_loader
		eq_cluster
		)

set( LIBS ${Boost_LIBRARIES} eqOgre )

add_executable( test_filestring test_filestring.cpp )
add_executable( test_settings test_settings.cpp )
#add_executable( test_dotscene test_dotscene.cpp )
add_executable( test_exceptions test_exceptions.cpp )
add_executable( test_tracking test_tracking.cpp )

target_link_libraries( test_filestring ${LIBS} )
target_link_libraries( test_settings ${LIBS} )
#target_link_libraries( test_dotscene ${LIBS} ${OGRE_LIBRARIES} )
target_link_libraries( test_exceptions ${LIBS} )
target_link_libraries( test_tracking ${LIBS} vrpn )

add_test( filestring ${PROJECT_BINARY_DIR}/test_filestring )
add_test( settings ${PROJECT_BINARY_DIR}/test_settings )
add_test( dotscene ${PROJECT_BINARY_DIR}/test_dotscene )
