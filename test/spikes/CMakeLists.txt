project( "spikes" CXX )

# Config file for spikes
set( VL_BUILD_CONF_DIR ${vl_BINARY_DIR} )
set( VL_DATA_PATH ${vl_SOURCE_DIR}/data )
set( VL_MEDIA_PATH ${vl_SOURCE_DIR}/../eqOgre-media )
configure_file( ${vl_SOURCE_DIR}/data/test_conf.xml
	${PROJECT_BINARY_DIR}/test_conf.xml )

# Plugins file
# Copyed to the vl root dir so that the plugins path are correct
if( WIN32 )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_REL} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins.cfg
		${vl_BINARY_DIR}/plugins.cfg )
	set( PLUGINS_DIR_DBG ${OGRE_PLUGIN_DIR_DBG} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins_d.cfg
		${vl_BINARY_DIR}/plugins_d.cfg )
elseif( UNIX )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_TMP} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins.cfg
		${vl_BINARY_DIR}/plugins.cfg )
endif()

add_executable( ogre_render ogre_render.cpp )
add_executable( stereo_render stereo_render.cpp )
#add_executable( pipes_test pipes_test.cpp )
add_executable( node_test node_test.cpp )
add_executable( tracking tracking.cpp )
add_executable( dotscene_loader dotscene_loader.cpp )

set( LIBS
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre
	)

target_link_libraries( ogre_render ${LIBS} )
target_link_libraries( stereo_render ${LIBS} )
#target_link_libraries( pipes_test ${LIBS} )
target_link_libraries( node_test ${LIBS} )
target_link_libraries( tracking ${LIBS} vrpn )
target_link_libraries( dotscene_loader ${LIBS} )