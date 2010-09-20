project( "spikes" CXX )

cmake_minimum_required(VERSION 2.6)

copy_config()

add_executable( spike_ogre_render 
	ogre_render.cpp
	../fixtures.hpp
	../debug.hpp
	)

add_executable( spike_stereo_render 
	stereo_render.cpp
	../fixtures.hpp
	../debug.hpp
	)

add_executable( spike_node_test 
	node_test.cpp
	../fixtures.hpp
	../debug.hpp
	)

add_executable( spike_tracking 
	tracking.cpp
	../debug.hpp
	)

add_executable( spike_dotscene
	dotscene_loader_demo.cpp
	../fixtures.hpp
	../debug.hpp
	)

add_executable( spike_listening_client
	listening_client.cpp
	../fixtures.hpp
	../debug.hpp
	)

add_executable( fake_tracking_server
	fake_tracking_server.cpp
	../tracking_fixture.hpp
	../debug.hpp
	)

set( LIBS
	${OPENGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	vrpn
	eqOgre
	)

target_link_libraries( spike_ogre_render ${LIBS} )
target_link_libraries( spike_stereo_render ${LIBS} )
target_link_libraries( spike_node_test ${LIBS} )
target_link_libraries( spike_tracking ${LIBS} )
target_link_libraries( spike_listening_client ${LIBS} )
target_link_libraries( spike_dotscene ${LIBS} )
target_link_libraries( fake_tracking_server vrpn )
