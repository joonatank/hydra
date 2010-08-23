project("eq_cluster_test" CXX)

# TODO these need 0-window and 1-window eqc files
# they should be copied here
configure_file( ${vl_SOURCE_DIR}/data/eqc/0-window.eqc
	${PROJECT_BINARY_DIR}/0-window.eqc )
configure_file( ${vl_SOURCE_DIR}/data/eqc/1-window.eqc
	${PROJECT_BINARY_DIR}/1-window.eqc )

add_executable( test_channel
	test_channel.cpp
	eqogre_fixture.hpp
	${vl_SOURCE_DIR}/vl/eq_cluster/channel.cpp
	${vl_SOURCE_DIR}/vl/eq_cluster/channel.hpp
	${vl_SOURCE_DIR}/vl/base/args.cpp
	)
	
add_executable( test_window
	test_window.cpp
	eqogre_fixture.hpp
	${vl_SOURCE_DIR}/vl/eq_cluster/window.cpp
	${vl_SOURCE_DIR}/vl/eq_cluster/window.hpp
	${vl_SOURCE_DIR}/vl/base/args.cpp
	)
	
add_executable( test_config
	test_config.cpp
	eqogre_fixture.hpp
	${vl_SOURCE_DIR}/vl/eq_cluster/config.cpp
	${vl_SOURCE_DIR}/vl/eq_cluster/config.hpp
	${vl_SOURCE_DIR}/vl/base/args.cpp
	)

set( LIBS
	${OPENGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	)

target_link_libraries( test_channel ${LIBS} )
target_link_libraries( test_window ${LIBS} )
target_link_libraries( test_config ${LIBS} )

