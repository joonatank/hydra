project("misc" CXX)

cmake_minimum_required(VERSION 2.8)

find_package(Boost COMPONENTS python REQUIRED )

find_package(PythonLibs REQUIRED )

find_package(PythonInterp REQUIRED )

include_directories( SYSTEM
	${TURTLE_INCLUDE_DIR}
	${PYTHON_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	)

add_executable( test_arguments
	test_arguments.cpp
	${eqOgre_SOURCE_DIR}/arguments.hpp
	)

add_executable( test_env
	test_env.cpp
	)

add_executable( python_embedded python_embedded.cpp )

target_link_libraries( python_embedded ${Boost_PYTHON_LIBRARIES} ${PYTHON_LIBRARIES} )
