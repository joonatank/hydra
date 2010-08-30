project( "spikes" CXX )

cmake_minimum_required(VERSION 2.6)

copy_config()

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
