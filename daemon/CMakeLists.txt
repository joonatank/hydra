#-------------------------------------------------------------------
# Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
# 2012-03
#
# This file is part of the CMake build system for Hydra
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#
#-------------------------------------------------------------------

project( "daemon" CXX )

cmake_minimum_required(VERSION 2.6)

set(DAEMON_SOURCES
	daemon.cpp
	daemon.hpp
	daemon_main.cpp
	${HydraMain_SOURCE_DIR}/base/sleep.hpp
	${HydraMain_SOURCE_DIR}/base/filesystem.hpp
	${HydraMain_SOURCE_DIR}/base/filesystem.cpp
	${HydraMain_SOURCE_DIR}/base/string_utils.cpp
	${HydraMain_SOURCE_DIR}/base/string_utils.hpp
	${HydraMain_SOURCE_DIR}/base/system_util.hpp
	${HydraMain_SOURCE_DIR}/base/system_util.cpp
	)

# Launcher
# Windows has a GUI for ease of management
# Linux version is designed to work as a daemon (started when system starts)
if(WIN32)
	add_executable(hydra_daemon WIN32
		daemon_gui.hpp
		daemon_gui.cpp
		res/resource.rc
		res/resource.hpp
		${DAEMON_SOURCES}
		)
	# TODO copy the incon file to output directory
else()
	add_executable(hydra_daemon
		${DAEMON_SOURCES}
		)
endif()


target_link_libraries(hydra_daemon
	${Boost_ASIO_LIBRARIES}
	)

install( TARGETS hydra_daemon DESTINATION bin CONFIGURATIONS Release )

