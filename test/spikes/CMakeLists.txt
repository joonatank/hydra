project( "spikes" CXX )

SET( TEST_SOURCES stereo_render ogre_render pipes_test )

add_executable( ogre_render ogre_render.cpp )
add_executable( stereo_render stereo_render.cpp )
add_executable( pipes_test pipes_test.cpp )
add_executable( node_test node_test.cpp )

set( LIBS
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre
	)

target_link_libraries( ogre_render ${LIBS} )
target_link_libraries( stereo_render ${LIBS} )
target_link_libraries( pipes_test ${LIBS} )
target_link_libraries( node_test ${LIBS} )

message( "Build type = ${CMAKE_BUILD_TYPE}" )
message( "CFLAGS debug = ${CMAKE_CXX_FLAGS_DEBUG}" )
message( "Compile definitions = ${COMPILE_DEFINITIONS}" )
message( "Compile definitions debug = ${COMPILE_DEFINITIONS_DEBUG}" )

#GET_TARGET_PROPERTY(LIB_NAME Foo LOCATION)
#GET_TARGET_PROPERTY(Bar_prefix Foo PREFIX)
#GET_TARGET_PROPERTY(Bar_suffix Foo SUFFIX)
#SET(NEW_LIB_NAME ${Bar_prefix}Bar${Bar_suffix})
 
#set( DATA_PATH ${vl_SOURCE_DIR}/data )
#set( RESOURCE_PATH ${vl_SOURCE_DIR}/data/resources/ )
#set( EQ_CONFIG 1-window.eqc 2-nodes.eqc 2-pipes.eqc )

#set( RESOURCE_TARGET  ${spikes_BINARY_DIR}/resources )

#message( "${DATA_PATH}/${EQ_CONFIG}" )
#add_custom_target(
#	resouces ALL
#	DEPENDS ${RESOURCE_PATH}
#	POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCE_PATH} ${RESOURCE_TARGET}
#)


#add_custom_command(
#	DEPENDS resources
#	OUTPUT ${RESOURCE_TARGET}
#	COMMAND ${CMAKE_COMMAND} -E create_symlink ${RESOURCE_PATH} ${RESOURCE_TARGET}
#)
