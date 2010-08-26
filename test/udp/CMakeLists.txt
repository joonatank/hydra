project(udp_test)

cmake_minimum_required(VERSION 2.6)

add_executable( test_udp_server
	${eqOgre_SOURCE_DIR}/udp/handler.cpp
	${eqOgre_SOURCE_DIR}/udp/print_handler.cpp
	${eqOgre_SOURCE_DIR}/udp/print_command.cpp
	${eqOgre_SOURCE_DIR}/udp/server.cpp
	${eqOgre_SOURCE_DIR}/udp/udp_conf.cpp
	server_main.cpp )

add_executable( test_udp_client
	${eqOgre_SOURCE_DIR}/udp/handler.cpp
	${eqOgre_SOURCE_DIR}/udp/print_handler.cpp
	${eqOgre_SOURCE_DIR}/udp/print_command.cpp
	${eqOgre_SOURCE_DIR}/udp/client.cpp
	${eqOgre_SOURCE_DIR}/udp/udp_conf.cpp
	client_main.cpp )

add_executable( test_udp
	${eqOgre_SOURCE_DIR}/udp/handler.cpp
	${eqOgre_SOURCE_DIR}/udp/print_handler.cpp
	${eqOgre_SOURCE_DIR}/udp/print_command.cpp
	${eqOgre_SOURCE_DIR}/udp/client.cpp
	${eqOgre_SOURCE_DIR}/udp/server.cpp
	${eqOgre_SOURCE_DIR}/udp/udp_conf.cpp
	test_udp.cpp )

add_executable( test_ogre_udp
	${eqOgre_SOURCE_DIR}/udp/handler.cpp
	${eqOgre_SOURCE_DIR}/udp/client.cpp
	${eqOgre_SOURCE_DIR}/udp/server.cpp
	${eqOgre_SOURCE_DIR}/udp/ogre_handler.cpp
	${eqOgre_SOURCE_DIR}/udp/udp_conf.cpp
	ogre_udp.cpp )

add_executable( test_udp_conf_parser
	${eqOgre_SOURCE_DIR}/udp/udp_conf.cpp
	test_udp_conf_parser.cpp )

target_link_libraries( test_udp_server ${Boost_LIBRARIES} )

target_link_libraries( test_udp_client ${Boost_LIBRARIES} )

target_link_libraries( test_udp ${Boost_LIBRARIES} )

target_link_libraries( test_udp_conf_parser ${Boost_LIBRARIES} )

target_link_libraries( test_ogre_udp
	${Boost_LIBRARIES}
	${OGRE_LIBRARIES}
	${OPENGL_LIBRARIES} )