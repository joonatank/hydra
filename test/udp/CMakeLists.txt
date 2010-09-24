project("udp_test" CXX)

cmake_minimum_required(VERSION 2.6)

copy_config()

add_executable( test_udp_server
	${eqOgre_SOURCE_DIR}/udp/print_command.cpp
	${eqOgre_SOURCE_DIR}/udp/print_command.hpp
	${eqOgre_SOURCE_DIR}/udp/server.cpp
	${eqOgre_SOURCE_DIR}/udp/server.hpp
	server_main.cpp )

add_executable( test_udp_client
	${eqOgre_SOURCE_DIR}/udp/client.cpp
	${eqOgre_SOURCE_DIR}/udp/client.hpp
	client_main.cpp )

add_executable( test_udp
	${eqOgre_SOURCE_DIR}/udp/client.cpp
	${eqOgre_SOURCE_DIR}/udp/server.cpp
	${eqOgre_SOURCE_DIR}/udp/command.hpp
	${eqOgre_SOURCE_DIR}/udp/client.hpp
	${eqOgre_SOURCE_DIR}/udp/server.hpp
	${eqOgre_SOURCE_DIR}/base/exceptions.hpp
	udp_fixtures.hpp
	test_udp.cpp 
	)

add_executable( test_ogre_udp
	udp_fixtures.hpp
	../test_helpers.hpp
	../ogre_fixtures.hpp
	test_ogre_udp.cpp
	)

add_executable( test_udp_conf_parser
	${eqOgre_SOURCE_DIR}/udp/server_config.cpp
	${eqOgre_SOURCE_DIR}/udp/server_config.hpp
	test_udp_conf_parser.cpp )

target_link_libraries( test_udp_server ${Boost_LIBRARIES} )

target_link_libraries( test_udp_client ${Boost_LIBRARIES} )

target_link_libraries( test_udp ${Boost_LIBRARIES} )

target_link_libraries( test_udp_conf_parser ${Boost_LIBRARIES} )

target_link_libraries( test_ogre_udp ${LIBS} )

