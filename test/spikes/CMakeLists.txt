#-------------------------------------------------------------------
# Joonatan Kuosa
# 2010-11
#
# This file is part of the CMake build system for eqOgre
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

project( "spikes" CXX )

cmake_minimum_required(VERSION 2.6)


add_executable( spike_udp_renderer
	udp_renderer.cpp
	client_fixtures.hpp
	)

target_link_libraries( spike_udp_renderer ${eqOgre_LIBRARY} )

