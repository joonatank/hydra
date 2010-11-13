add_executable( test_arguments
	test_arguments.cpp
	${eqOgre_SOURCE_DIR}/arguments.hpp
	)

add_executable( test_env
	test_env.cpp
	)

add_executable( python_embedded python_embedded.cpp )

target_link_libraries( python_embedded ${Boost_PYTHON_LIBRARIES} ${PYTHON_LIBRARIES} )
