project("tests" CXX)

cmake_minimum_required(VERSION 2.6)

include_directories( util )

subdirs( util math eq_graph spikes eq_ogre synchronization dotscene_loader
	vl_dotscene_loader )
#subdirs( util math eq_graph spikes eq_ogre synchronization
#	vl_dotscene_loader )

set( LIBS ${Boost_LIBRARIES} eqOgre )

set( VL_DATA_PATH ${vl_SOURCE_DIR}/data )
set( VL_MEDIA_PATH ${vl_SOURCE_DIR}/../eqOgre-media )

configure_file( ${vl_SOURCE_DIR}/data/test_conf.xml
	${PROJECT_BINARY_DIR}/test_conf.xml )
configure_file( ${vl_SOURCE_DIR}/data/inv_conf.xml
	${PROJECT_BINARY_DIR}/inv_conf.xml )

add_executable( test_settings test_settings.cpp )
target_link_libraries( test_settings ${LIBS} )
add_test( settings ${PROJECT_BINARY_DIR}/test_settings )
