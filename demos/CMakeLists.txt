project( "demos" CXX )

cmake_minimum_required(VERSION 2.8)

message( "Thread libraries = ${CMAKE_THREAD_LIBS_INIT}" )
message( "VRPN libraries = ${VRPN_LIBRARY}" )

add_executable( fake_tracking_server
	fake_tracking_server.cpp
	)

target_link_libraries( fake_tracking_server ${VRPN_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} )


add_executable( spike_listening_client
	listening_client.cpp
	)

target_link_libraries( spike_listening_client ${LIBS} )
copy_start_script( spike_listening_client )
copy_extras()