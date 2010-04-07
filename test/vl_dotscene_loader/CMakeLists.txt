project( "vl_dotscene_loader" CXX )

add_executable( loader main.cpp dotscene_loader.cpp )

set( LIBS
	${OpenGL_LIBRARIES}
	${OGRE_LIBRARIES}
	${Boost_LIBRARIES}
	${EQ_LIBRARIES}
	eqOgre
	)

target_link_libraries( loader ${LIBS} )

