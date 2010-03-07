SET( TEST_SOURCES math )

FOREACH( source ${TEST_SOURCES} )
	add_executable( test_${source}
		test_${source}.cpp )
	target_link_libraries( test_${source} boost_unit_test_framework )
ENDFOREACH( source )
