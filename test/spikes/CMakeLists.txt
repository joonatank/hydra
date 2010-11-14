project( "spikes" CXX )

cmake_minimum_required(VERSION 2.8)


# TODO this should be moved to demos
add_executable( spike_udp_renderer
	udp_renderer.cpp
	client_fixtures.hpp
	)

target_link_libraries( spike_udp_renderer ${LIBS} )


