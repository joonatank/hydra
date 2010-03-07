project("tests" CXX)

cmake_minimum_required(VERSION 2.8)

message( "Building tests" )

#${CMAKE_CURRENT_SOURCE_DIR}/eq_cluster
subdirs( math eq_graph )
