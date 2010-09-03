project( "spikes" CXX )

cmake_minimum_required(VERSION 2.6)

copy_config()

SET( HEADERS
	../fixtures.hpp
	../debug.hpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_root.hpp
	${eqOgre_SOURCE_DIR}/base/args.hpp
	${eqOgre_SOURCE_DIR}/base/exceptions.hpp
	${eqOgre_SOURCE_DIR}/math/conversion.hpp
	${eqOgre_SOURCE_DIR}/math/math.hpp
	${eqOgre_SOURCE_DIR}/settings.hpp
	${eqOgre_SOURCE_DIR}/dotscene_loader.hpp
	)

set( SOURCES 
	${eqOgre_SOURCE_DIR}/settings.cpp
	${eqOgre_SOURCE_DIR}/base/args.cpp
	${eqOgre_SOURCE_DIR}/eq_ogre/ogre_root.cpp
	${eqOgre_SOURCE_DIR}/dotscene_loader.cpp
	)

add_executable( spike_ogre_render 
	ogre_render.cpp
	${SOURCES}
	${HEADERS}
	)

add_executable( spike_stereo_render 
	stereo_render.cpp
	${SOURCES}
	${HEADERS}
	)

add_executable( spike_node_test 
	node_test.cpp
	${SOURCES}
	${HEADERS}
	)

add_executable( spike_tracking 
	tracking.cpp
	${SOURCES}
	${HEADERS}
	${eqOgre_SOURCE_DIR}/vrpn_tracker.hpp
	${eqOgre_SOURCE_DIR}/vrpn_tracker.cpp
	${eqOgre_SOURCE_DIR}/tracker.hpp
	)

add_executable( spike_dotscene
	dotscene_loader_demo.cpp
	${eqOgre_SOURCE_DIR}/tracker.hpp
	${eqOgre_SOURCE_DIR}/vrpn_tracker.hpp
	${eqOgre_SOURCE_DIR}/vrpn_tracker.cpp
	${eqOgre_SOURCE_DIR}/fake_tracker.hpp
	${eqOgre_SOURCE_DIR}/fake_tracker.cpp
	${eqOgre_SOURCE_DIR}/eq_cluster/config.cpp
	${eqOgre_SOURCE_DIR}/eq_cluster/config.hpp
	${SOURCES}
	${HEADERS}
	)

add_executable( fake_tracking_server
	fake_tracking_server.cpp
	../tracking_fixture.hpp
	)

set( LIBS
	${OPENGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	)

target_link_libraries( spike_ogre_render ${LIBS} )
target_link_libraries( spike_stereo_render ${LIBS} )
target_link_libraries( spike_node_test ${LIBS} )
target_link_libraries( spike_tracking ${LIBS} vrpn )
target_link_libraries( spike_dotscene ${LIBS} vrpn )
target_link_libraries( fake_tracking_server vrpn )
