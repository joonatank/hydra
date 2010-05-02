project("tests" CXX)

cmake_minimum_required(VERSION 2.6)

include_directories( util )

subdirs( util math eq_graph spikes eq_ogre synchronization dotscene_loader
	vl_dotscene_loader )
#subdirs( util math eq_graph spikes eq_ogre synchronization
#	vl_dotscene_loader )

set( LIBS ${Boost_LIBRARIES} eqOgre )

add_executable( test_settings test_settings.cpp )
target_link_libraries( test_settings ${LIBS} )
add_test( settings ${PROJECT_BINARY_DIR}/test_settings )
