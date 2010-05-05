project( "math_tests" CXX )

SET( TEST_SOURCES math )

FOREACH( source ${TEST_SOURCES} )
	add_executable( test_${source} test_${source}.cpp )
	target_link_libraries( test_${source} ${Boost_LIBRARIES} )
	add_test( ${source} ${PROJECT_BINARY_DIR}/test_${source} )
ENDFOREACH( source )
