#ifndef EQ_OGRE_CONFIG_PYTHON_HPP
#define EQ_OGRE_CONFIG_PYTHON_HPP

#include "config.hpp"
#include "scene_node.hpp"

#include <boost/python.hpp>

namespace python = boost::python;

class ConfigWrapper
{
public :
	ConfigWrapper( eqOgre::Config *config )
		: _config(config)
	{}

	void addEvent( void )
	{
		std::cout << "C++ : config = " << _config << " : ConfigWrapper::addEvent" << std::endl;
	}

	void addSceneNode( void )
	{
		std::cout << "C++ : config = " << _config  << " : ConfigWrapper::addSceneNode" << std::endl;
	}

	void test_print( void )
	{
		std::cout << "C++ : config = " << _config << " : ConfigWrapper::test_print" << std::endl;
	}
private :
	eqOgre::Config *_config;
};

class Test
{
public :
	void test( void )
	{
		std::cout << "C++ Test::test" << std::endl;
	}
};

void test_print( void )
{
	std::cout << "C++ test_print" << std::endl;
}

BOOST_PYTHON_MODULE(eqOgre_python)
{
	// NOTE renaming classes works fine
	// TODO registering objects is not working when using modules
	// or I don't know how to register the modules first
	// init the interpreter and then start calling functions one by one
	python::class_<ConfigWrapper>("ConfigWrapper", python::no_init)
		.def("addEvent", &ConfigWrapper::addEvent)
		.def("addSceneNode", &ConfigWrapper::addSceneNode)
		.def("test_print", &ConfigWrapper::test_print)
	;

	python::class_<eqOgre::SceneNode>("SceneNode")
//		.def("getName", &eqOgre::SceneNode::getName)
//		.def("setName", &eqOgre::SceneNode::setName)
//		.def("getPosition", &eqOgre::SceneNode::getPosition)
	;

	python::class_<Test>("Test")
		.def("test", &Test::test )
	;

	// NOTE renaming functions works fine
	python::def("test", test_print);
}

#endif