project("tests" CXX)

cmake_minimum_required(VERSION 2.6)

include_directories( util )

subdirs( util math eq_graph spikes eq_ogre synchronization dotscene_loader )

set( LIBS ${Boost_LIBRARIES} eqOgre )

# Config file for spikes
set( VL_DATA_PATH ${vl_SOURCE_DIR}/data )
set( VL_MEDIA_PATH ${vl_SOURCE_DIR}/../eqOgre-media )
configure_file( ${vl_SOURCE_DIR}/data/test_conf.xml
	${PROJECT_BINARY_DIR}/test_conf.xml )

# Plugins file
#set( PLUGINS_DIR ${OGRE_PLUGIN_DIR_REL} )
message( "OGRE plugin dir = ${OGRE_PLUGIN_DIR_REL} = ${OGRE_PLUGIN_DIR_DBG} = ${OGRE_PLUGIN_DIR_TMP}" )

if( WIN32 )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_REL} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins.cfg
		${PROJECT_BINARY_DIR}/plugins.cfg )
	set( PLUGINS_DIR_DBG ${OGRE_PLUGIN_DIR_DBG} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins_d.cfg
		${PROJECT_BINARY_DIR}/plugins_d.cfg )
elseif( UNIX )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_TMP} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins.cfg
		${PROJECT_BINARY_DIR}/plugins.cfg )
endif()

add_executable( test_filestring test_filestring.cpp )
add_executable( test_settings test_settings.cpp )
add_executable( test_dotscene test_dotscene.cpp )

target_link_libraries( test_filestring ${LIBS} )
target_link_libraries( test_settings ${LIBS} )
target_link_libraries( test_dotscene ${LIBS} ${OGRE_LIBRARIES} )

add_test( filestring ${PROJECT_BINARY_DIR}/test_filestring )
add_test( settings ${PROJECT_BINARY_DIR}/test_settings )
add_test( dotscene ${PROJECT_BINARY_DIR}/test_dotscene )
