# This is the eqOgre test app
TEMPLATE = app

CFLAGS += -Wno-deprecated

CONFIG += debug warn_on thread
CONFIG -= qt

# We use advanced c++/processor features, can be disabled if necessary
CONFIG += exceptions rtti sse2
# For now we don not use qt for anything but later we might use signals/slots
# or networking from qt.
QT -= gui

#message(Building with config $$(CONFIG) )

INCLUDEPATH += /usr/local/include/OGRE /usr/local/include/raknet
LIBS += -L/usr/local/lib/
# Using Ogre as rendering engine
LIBS += -lOgreMain
# Using equalizer for distributed render window creation and VR features
LIBS +=-leq -leqserver -lGLEW
# Using RakNet for distributing commands across nodes
LIBS += -lRakNet
#LIBS += -L/usr/local/lib/ -lOgreMain -leq -leqserver -lGLEW -lRakNet

DESTDIR = ../bin/

MOC_DIR = ../build/eqOgre/moc
OBJECTS_DIR = ../build/eqOgre/obj

HEADERS = channel.hpp\
		  client.hpp\
		  config.hpp\
		  node.hpp\
		  nodeFactory.hpp\
		  pipe.hpp\
		  window.hpp\
		  graph/entity.hpp\
		  graph/scene_node.hpp\
		  graph/scene_manager.hpp\
		  graph/root.hpp\
		  graph/movable_object.hpp\
		  graph/render_window.hpp\
		  graph/viewport.hpp\
		  graph/camera.hpp\
		  ogre/ogre_entity.hpp\
		  ogre/ogre_movable_object.hpp\
		  ogre/ogre_scene_node.hpp\
		  ogre/ogre_scene_manager.hpp\
		  ogre/ogre_root.hpp\
		  ogre/ogre_render_window.hpp\
		  ogre/ogre_viewport.hpp\
		  ogre/ogre_camera.hpp\
		  utility.hpp\
		  fifo_buffer.hpp\
		  command.hpp\
		  command_factory.hpp\
		  server_command.hpp\
		  exceptions.hpp\
		  cmd_network.hpp\
		  udp_server.hpp

SOURCES = channel.cpp\
		  client.cpp\
		  config.cpp\
		  node.cpp\
		  pipe.cpp\
		  window.cpp\
		  graph/scene_node.cpp\
		  graph/scene_manager.cpp\
		  graph/root.cpp\
		  ogre/ogre_scene_manager.cpp\
		  ogre/ogre_scene_node.cpp\
		  ogre/ogre_entity.cpp\
		  object_params.cpp\
		  main.cpp\
		  server_command.cpp\
		  udp_server.cpp
