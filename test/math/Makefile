all : vector_test quaternion_test

vector_test: vector_test.cpp ../../math/vector.hpp
	g++ -Wall -pedantic -o $@ $^

quaternion_test: quaternion_test.cpp ../../math/quaternion.hpp
	g++ -Wall -pedantic -o $@ $^

clean :
	rm -f vector_test quaternion_test
