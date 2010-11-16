project( "spikes" CXX )

cmake_minimum_required(VERSION 2.8)


# TODO this should be moved to demos
# TODO should be updated from windows
#add_executable( spike_udp_renderer
#	udp_renderer.cpp
#	client_fixtures.hpp
#	)

#target_link_libraries( spike_udp_renderer ${LIBS} )

find_package( Bullet REQUIRED )

message( "Bullet include dir = ${BULLET_INCLUDE_DIRS}" )
message( "Bullet libraries = ${BULLET_LIBRARIES}" )

include_directories(
	${BULLET_INCLUDE_DIRS}
	${BULLET_INCLUDE_DIRS}/bullet
	)

add_executable( spike_bullet_test
	bullet_test.cpp
	)

target_link_libraries( spike_bullet_test ${BULLET_LIBRARIES} )

add_executable( spike_bullet
	bullet_spike.cpp
	)

target_link_libraries( spike_bullet ${BULLET_LIBRARIES} ${LIBS} )

copy_start_script( spike_bullet )