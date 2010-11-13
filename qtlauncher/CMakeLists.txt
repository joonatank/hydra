project("qtlauncher" CXX)

cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(Qt4 REQUIRED)

include_directories( )
include( ${QT_USE_FILE} )

ADD_DEFINITIONS(${QT_DEFINITIONS})


set( SOURCES
	${eqOgre_SOURCE_DIR}/base/envsettings.cpp
	${eqOgre_SOURCE_DIR}/base/projsettings.cpp
	${eqOgre_SOURCE_DIR}/base/filesystem.cpp
	mainwindow.cpp
	envcontrol.cpp
	projectcontrol.cpp
	projpage.cpp
	main.cpp
	)

set( VL_HEADERS
	${eqOgre_SOURCE_DIR}/base/envsettings.hpp
	${eqOgre_SOURCE_DIR}/base/projsettings.hpp
	${eqOgre_SOURCE_DIR}/base/filesystem.hpp
	${eqOgre_SOURCE_DIR}/base/string_utils.hpp
	${eqOgre_SOURCE_DIR}/base/exceptions.hpp
	${eqOgre_SOURCE_DIR}/base/rapidxml.hpp
	${eqOgre_SOURCE_DIR}/base/rapidxml_print.hpp
	)

set( UI_HEADERS
	projpage.h
	mainwindow.h
	envcontrol.h
	projectcontrol.h
	)

set( RESOURCES
	)

set( FORMS
	mainwindow.ui
	envcontrol.ui
	projpage.ui
	)

QT4_WRAP_CPP( HEADERS_MOC ${UI_HEADERS} )
QT4_WRAP_UI( FORMS_HEADERS ${FORMS} )
QT4_ADD_RESOURCES( RESOURCES_RCC ${RESOURCES} )

add_executable( qtlauncher
	${SOURCES}
	${UI_HEADERS}
	${VL_HEADERS}
	${HEADERS_MOC}
	${FORMS_HEADERS}
	${RESOURCES_RCC}
	)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

target_link_libraries( qtlauncher
	${QT_LIBRARIES}
	${Boost_SYSTEM_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARIES}
	)
