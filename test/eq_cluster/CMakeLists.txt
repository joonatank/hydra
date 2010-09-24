project("eq_cluster_test" CXX)

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
	../settings_fixtures.hpp
	)

target_link_libraries( test_channel ${LIBS} )
target_link_libraries( test_window ${LIBS} )
target_link_libraries( test_config ${LIBS} )
target_link_libraries( test_init_data ${LIBS} )

