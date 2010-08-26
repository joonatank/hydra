project( "spikes" CXX )

# Config file for spikes
if( WIN32 )
	set( VL_BUILD_CONF_DIR ${PROJECT_BINARY_DIR}/Debug )
else( )
	set( VL_BUILD_CONF_DIR ${PROJECT_BINARY_DIR} )
endif()

set( VL_DATA_PATH ${vl_SOURCE_DIR}/data )
set( VL_MEDIA_PATH ${vl_SOURCE_DIR}/../eqOgre-media )
configure_file( ${vl_SOURCE_DIR}/data/test_conf.xml
	${PROJECT_BINARY_DIR}/test_conf.xml )

# Plugins file
# Copyed to the vl root dir so that the plugins path are correct
if( WIN32 )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_REL} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins.cfg
		${PROJECT_BINARY_DIR}/Release/plugins.cfg )
	set( PLUGINS_DIR_DBG ${OGRE_PLUGIN_DIR_DBG} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins_d.cfg
		${PROJECT_BINARY_DIR}/Debug/plugins.cfg )
else( )
	set( PLUGINS_DIR_REL ${OGRE_PLUGIN_DIR_TMP} )
	configure_file( ${vl_SOURCE_DIR}/data/plugins.cfg
		${PROJECT_BINARY_DIR}/plugins.cfg )
endif()

SET( HEADERS
	../fixtures.hpp 
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_root.hpp
	${eqOgre_SOURCE_DIR}/base/args.hpp
	${eqOgre_SOURCE_DIR}/base/exceptions.hpp
	${eqOgre_SOURCE_DIR}/math/conversion.hpp
	${eqOgre_SOURCE_DIR}/settings.hpp
	${eqOgre_SOURCE_DIR}/dotscene_loader.hpp
	)

set( SOURCES 
	${eqOgre_SOURCE_DIR}/settings.cpp
	${eqOgre_SOURCE_DIR}/base/args.cpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_root.cpp
	${eqOgre_SOURCE_DIR}/dotscene_loader.cpp
	)

add_executable( ogre_render 
	ogre_render.cpp
	${SOURCES}
	${HEADERS}
	)

add_executable( stereo_render 
	stereo_render.cpp
	${SOURCES}
	${HEADERS}
	)

add_executable( node_test 
	node_test.cpp
	${SOURCES}
	${HEADERS}
	)

add_executable( tracking 
	tracking.cpp
	${SOURCES}
	${HEADERS}
	)

add_executable( dotscene_loader_tracking
	dotscene_loader_tracking.cpp
	${SOURCES}
	${HEADERS}
	)

add_executable( dotscene_loader_demo
	dotscene_loader_demo.cpp
	${SOURCES}
	${HEADERS}
	)

set( LIBS
	${OPENGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	)

target_link_libraries( ogre_render ${LIBS} )
target_link_libraries( stereo_render ${LIBS} )
target_link_libraries( node_test ${LIBS} )
target_link_libraries( tracking ${LIBS} vrpn )
target_link_libraries( dotscene_loader_tracking ${LIBS} vrpn )
target_link_libraries( dotscene_loader_demo ${LIBS} )
