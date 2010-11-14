project("eq_cluster_test" CXX)

# TODO this should use same Settings stucture as the rest of them i.e. it needs
# Either a start script or a config file,
# Start scripts are bit difficult for automated testing
# So a config file would be better
# Need to add boost::program_options first though
#copy_config()

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

target_link_libraries( test_channel ${LIBS} )
target_link_libraries( test_window ${LIBS} )
target_link_libraries( test_config ${LIBS} )


