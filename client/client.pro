# This is the client test app
TEMPLATE = app

CFLAGS += -Wno-deprecated

CONFIG += debug warn_on thread
CONFIG -= qt

# We use advanced c++/processor features, can be disabled if necessary
CONFIG += exceptions rtti sse2
# For now we don not use qt for anything but later we might use signals/slots
# or networking from qt.
#QT -= gui

# For networking we use RakNet
INCLUDEPATH += /usr/local/include/raknet
INCLUDEPATH += /usr/local/include/OGRE
# We need Command classes from eqOgre
INCLUDEPATH += ../eqOgre
LIBS += -L/usr/local/lib/ -lRakNet
# For now we link to Ogre, later introduce our own vectors, quats and strings
LIBS += -lOgreMain
# For now link to equalizer for EQASSERT
LIBS += -leq

DESTDIR = ../bin/

DEPENDPATH= ../eqOgre

MOC_DIR = ../build/client/moc
OBJECTS_DIR = ../build/client/obj

HEADERS = command.hpp\
		  exceptions.hpp\
		  cmd_network.hpp

SOURCES = main.cpp\
		  command.cpp
