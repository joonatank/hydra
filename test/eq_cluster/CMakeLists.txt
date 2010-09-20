project("eq_cluster_test" CXX)

# TODO these need 0-window and 1-window eqc files
# they should be copied here
#configure_file( ${vl_SOURCE_DIR}/data/eqc/0-window.eqc
	#	${PROJECT_BINARY_DIR}/0-window.eqc )
#configure_file( ${vl_SOURCE_DIR}/data/eqc/1-window.eqc
	#	${PROJECT_BINARY_DIR}/1-window.eqc )

copy_config()

add_executable( test_channel
	test_channel.cpp
	eqogre_fixture.hpp
	)
	
add_executable( test_window
	test_window.cpp
	eqogre_fixture.hpp
	)
	
add_executable( test_config
	test_config.cpp
	eqogre_fixture.hpp
	)

add_executable( test_init_data
	test_init_data.cpp
	../fixtures.hpp
	)

set( LIBS
	${OPENGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre
	)

target_link_libraries( test_channel ${LIBS} vrpn )
target_link_libraries( test_window ${LIBS} vrpn )
target_link_libraries( test_config ${LIBS} )
target_link_libraries( test_init_data ${LIBS} )

